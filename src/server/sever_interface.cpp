#include <iostream>          // std::cout, std::cin, std::endl, std::string, std::getline(), std::max()
#include <exception>         // std::runtime_error
#include <semaphore.h>       // sem_t,  sem_post()
#include <thread>            // std::thread: join(), joinable()
#include <sys/socket.h>      // socketpair(), SOCK_STREAM
#include <unistd.h>          // write(), select(), FD_ZERO, FD_SET, FD_ISSET, STDIN_FILENO
#include <cstring>           // strerror()
#include <errno.h>           // errno
#include "server.h"          // Server

#include "sever_interface.h" // прототипы всех функций, описанных в этом файле


Server_interface::Server_interface(Server &server, sem_t &sem) : server(server), sem(sem)
{
    // создаём сокет, необходимый для взаимодействия с циклом:
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, socket_stop) == -1) {
        throw std::runtime_error("[Server_interface] start_listen() " + std::string(strerror(errno)));
    }
    is_on_run = false;
}


void Server_interface::start_listen()
{
    _listen_thread = std::thread(&Server_interface::on_listen, this);
    is_on_run = true;
}


Server_interface::~Server_interface()
{
    if (is_on_run) {
        Server_interface::stop_listen();
    }
}


void Server_interface::stop_listen()
{
    // будим поток-обработчик
    if (write(socket_stop[0], stop_message, sizeof(stop_message)) < 0) {
        throw std::runtime_error("Failed to wakeup listener thread");
    }
    // ждем завершения потока
    if(_listen_thread.joinable()){
        _listen_thread.join();
    }
    is_on_run = false;
    std::cout << "interface is stopped" << std::endl;
}


void Server_interface::on_listen()
{
    while (true) {
        fd_set readfds;                    // множество читаемых дескрипторов

        FD_ZERO(&readfds);                 // очищаем множество
        FD_SET(STDIN_FILENO, &readfds);    // добавляем во множество дескриптор слушающего сокета
        FD_SET(socket_stop[1], &readfds);  // добавляем во множество служебный дескриптор

        int max_sfd = std::max(STDIN_FILENO, socket_stop[1]);            // первый параметр системного вызова select()
        if (select(max_sfd + 1, &readfds, nullptr, nullptr, nullptr) == -1) {
            throw std::runtime_error("[Server_interface] select() " + std::string(strerror(errno)));
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            std::cout << "listener thread got message" << std::endl;
            std::string message;
            std::getline(std::cin, message);
            if (message == "\\exit") {
                sem_post(&sem); // с консоли пришел сигнал остановиться
                break;
            }
        }

        if (FD_ISSET(socket_stop[1], &readfds)) {
            std::cout << "listener thread is stopped" << std::endl;
            break;
        }
    } // end while (true)
}
