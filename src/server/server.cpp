#include <iostream>     // std::max, std::cout, std::endl, std::string
#include <list>         // std::list: clear(), push_back(), remove()
#include <exception>    // std::runtime_error: what()
#include <unistd.h>     // select(), write(), close(), FD_ZERO, FD_SET, FD_ISSET, gethostname()
#include <sys/socket.h> // semd(), recv(), socket(), bind(), connect(), listen(), accept(), socketpair(), setsockopt()
#include <cstring>      // strerror(), strlen(), strcat(), memset(). memcpy()
#include <errno.h>      // errno
#include <netdb.h>      // gethostbyname()
#include <arpa/inet.h>  // inet_ntoa()
#include <fcntl.h>      // fcntl()
#include <stdarg.h>     // va_list, va_start(), va_arg()
#include <thread>       // std::thread: join(), joinable()
#include <analyze/analyze.h>    // Analyze: start()
#include <exeption/exception.h> // std::runtime_error

#include "server.h"     // прототипы всех функций, описанных в этом файле


Server::Server(int port, int queue_len) : port(port), queue_len(queue_len)
{
    // создаём сокет, необходимый для взаимодействия с обрабатывающим циклом:
    if (socketpair(PF_LOCAL, SOCK_STREAM, 0, socket_stop) == -1) {
        throw std::runtime_error("[User_interface] start_listen() " + std::string(strerror(errno)));
    }
    is_on_run = false;
}


Server::~Server()
{
    if (is_on_run) {
        Server::stop();
    }
}


void Server::start()
{
    std::cout << "[server] RUN" << std::endl;
    // создаём и привязываем сокет: socket(), bind(); выдаём информацию о сервере: IP, порт
    install_server();
    // запускаем обработку соединений
    // делаем это в треде, чтоб из метода start возвращаться раньше, чем никогда
    _work_thread = std::thread(&Server::on_run, this);
    is_on_run = true;
}


void Server::stop()
{
    std::cout << "server is stopped" << std::endl;
    send2all("\\exit", NULL);
    // будим поток-обработчик
    if (write(socket_stop[0], stop_message, sizeof(stop_message)) < 0) {
        throw std::runtime_error("Failed to wakeup worker thread");
    }
    // ждем пока обработчик остановится
    if (_work_thread.joinable()) {
        _work_thread.join();
    }

    // закрываем дескриптор сокета:
    if (close(listener) == -1) {
        std::cerr << "[client] close() " << strerror(errno) << std::endl;
    }
    // сразу выводим сообщение об ошибке в поток cerr

    // закрываем дескрипторы кленитов
    for (auto client : client_connections) {
        close(client);
    }

    client_connections.clear();
    is_on_run = false;
}


void Server::on_run()
{
    // ожидание и приём клиентских соединений:
    if (listen(listener, queue_len) == -1) {
        throw std::runtime_error("[server] listen() " + std::string(strerror(errno)));
    }
    // основной рабочий цикл:
    while (true) {
        fd_set readfds;                    // множество читаемых дескрипторов

        int max_sfd = std::max(socket_stop[1], listener); // первый параметр системного вызова select()

        FD_ZERO(&readfds);                 // очищаем множество
        FD_SET(listener, &readfds);        // добавляем во множество дескриптор слушающего сокета
        FD_SET(socket_stop[1], &readfds);  // добавляем во множество служебный дескриптор
        // добавляем во множество дескрипторы сокетов клиентов:
        for (auto user_sockfd :client_connections) {
            FD_SET(user_sockfd, &readfds);
            if (user_sockfd > max_sfd) {
                max_sfd = user_sockfd;
            }
        }

        if (select(max_sfd + 1, &readfds, nullptr, nullptr, nullptr) == -1) {
            throw std::runtime_error("[server] select() " + std::string(strerror(errno)));
        }

        // пришёл сигнал остановиться:
        if (FD_ISSET(socket_stop[1], &readfds)) {
            std::cout << "worker thread is stopped" << std::endl;
            break;
        }

        // пришёл новый запрос на соединение с сервером:
        if (FD_ISSET(listener, &readfds)) {
            accept_client();
        }
        // пришёл SQL-запрос от клиента:
        for (auto user_sockfd : client_connections) {
            if (FD_ISSET(user_sockfd, &readfds)) {
                recv_msg_from(user_sockfd);
                break; // необходим, т.к. может измениться БД и поведение цикла станет неопредлённым
            }
        }
    } // end while
}


void Server::install_server()
{
    // создаём неблокирующий слушающий сокет:
    if ((listener = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        throw std::runtime_error("[server] socket() " + std::string(strerror(errno)));
    }
    // получаем имя машины:
    char hostname[255]; // SUSv2 гарантирует, что длина имени машины ограничивается 255-ми байтами
    if (gethostname(hostname, 255) == -1) {
        throw std::runtime_error("[server] gethostname() " + std::string(strerror(errno)));
    }
    // получаем информацию о машине в сети:
    struct hostent *hs;
    if ((hs = gethostbyname(hostname)) == nullptr) {
        throw std::runtime_error("[server] gethostbyname() " + std::string(strerror(errno)));
    }
    // заполняем структуру сервера:
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    memcpy(&local_addr.sin_addr, *hs->h_addr_list, hs->h_length);
    // для избежания "залипания" TCP-порта по завершению сервера:
    int opt = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error("[server] setsockopt() " + std::string(strerror(errno)));
    }
    // связывание сокета с адресом:
    if (bind(listener, (struct sockaddr *) &local_addr, sizeof(local_addr)) == -1) {
        throw std::runtime_error("[server] bind() " + std::string(strerror(errno)));
    }
    // выводим информация о сервере:
    std::cout << std::endl
              << "IP  : " << inet_ntoa(local_addr.sin_addr) << std::endl
              << "port: " << port << std::endl
              << std::endl
              << " to shutdown the server enter the command \\exit"
              << std::endl
              << std::endl;
}


void Server::accept_client()
{
    // устанавливаем соединение с клиентом:
    int client_sockfd = accept(listener, nullptr, nullptr); // дескриптор сокета клиента
    if (client_sockfd == -1) {
        throw std::runtime_error("[server] accept() " + std::string(strerror(errno)));
    }
    // устанавливаем неблокирующий дескриптор сокета:
    if (fcntl(client_sockfd, F_SETFL, O_NONBLOCK) == -1) {
        throw std::runtime_error("[server] fcntl() " + std::string(strerror(errno)));
    }
    // добавляем клиента в список подключенных клиентов:
    client_connections.push_back(client_sockfd);
    std::cout << "***Customer [" << client_sockfd << "] has joined" << std::endl;
}


void Server::recv_msg_from(int user_sockfd)
{
    char message[MSG_LEN] = "\0"; // буфер сообщения клиента
    std::string command;          // полный клиентский запрос
    int nrecv = -1;               // количество прочитанных сообщений

    // получаем сообщение от клиента:
    do {
        if ((nrecv = recv(user_sockfd, message, MSG_LEN, 0)) == -1) {
            throw std::runtime_error("[server] recv() " + std::string(strerror(errno)));
        }
        command += message;
    } while (nrecv == MSG_LEN);

/* DEBUG */ /* std::cout << command << std::endl; */

    if (command == "\\exit") { // клиент отсоеденился от сервера
        close(user_sockfd);    // закрываем дескриптор
        client_connections.remove(user_sockfd); // удаляем его из базы данных
        std::cout << "***Customer [" << user_sockfd << "] disconnected" << std::endl;
    }
    else { // обрабатываем клиентский запрос
        try {
            // запускаем анализ команды
            Analyze analyze = Analyze(user_sockfd, command);
            analyze.start();
            // если вышли, то анализ завершился успешно
            std::string table_result = std::string("[v] Done") + analyze.get_table_text();
            if (send(user_sockfd, table_result.c_str(), table_result.length(), 0) == -1) {
                throw std::runtime_error("[server] send() " + std::string(strerror(errno)));
            }
        }
        catch (std::exception &err) {
            // вылетело исключение => ошибка анализа, уведомляем клиента:
            if (send(user_sockfd, err.what(), strlen(err.what()), 0) == -1) {
                throw std::runtime_error("[server] send() " + std::string(strerror(errno)));
            }
        }
    }
}


void Server::send2all(const char *message1, ...) const
{
    va_list argPtr;
    const char *temp = message1;
    int msglen = 0; // длина сообщения
    // считаем длину сообщения, заданного аргументами:
    for (va_start(argPtr, message1); temp != nullptr; temp = va_arg(argPtr, char *)) {
        msglen += strlen(temp);
    }

    char buf[msglen + 1]; // буфер сообщения
    *buf = '\0';          // необходимо для корректной работы с strcat()
    // записываем все аргументы в общий буфер:
    for (temp = message1, va_start(argPtr, message1); temp != nullptr; temp = va_arg(argPtr, char *)) {
        strcat(buf, temp);
    }

    // рассылаем сообщение всем клиентам:
    for (auto user_sockfd : client_connections) {
        if (send(user_sockfd, buf, msglen + 1, 0) == -1) {
            throw std::runtime_error("[server] send() " + std::string(strerror(errno)));
        }
    }
}
