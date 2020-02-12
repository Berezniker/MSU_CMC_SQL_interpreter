#include <iostream>     // std::cerr, std::endl, std::string.clear()
#include <string.h>     // strerror()
#include <errno.h>      // errno
#include <signal.h>     // signal(), SIGINT, SIGTSTP
#include <unistd.h>     // _exit(), fd_set, select(), FD_ZERO(), FD_SET(), FD_ISSET(), STDIN_FILENO
#include <sys/types.h>
#include <client/client.h>       // check_argument(), invite(), exec_command(), recv_msg(), sock_off(), print_from_file()
#include <exeption/exception.h>  // FatalError, what(), isfatal()

#define WELCOME_FILE _cmake_assistive_dir"/welcome.txt" // имя файла, содержащего приветственное сообщение


int client_sockfd = -1;          // дескриптор сокета клиента
std::string command;             // буфер команд/запроса пользователя
/* Note: переменные глобальные, т.к. нужны в обработчике сигналов handler()
                                        для корректного завершения работы программы */

/**
 * [handler: SIGINT(<Ctrl+C>) and SIGTSTP(<Ctrl+Z>) signals handler]
 * [         and program shutdown with code <signum>               ]
 */
void handler(int signum);


int main(int argc, char *argv[])
{
    // устанавливаем обработчик сигналов <Ctrl+C> и <Ctrl+Z>
    signal(SIGINT,  handler);
    signal(SIGTSTP, handler);

    // работа с параметрами, заданными в командной строки при запуске модуля клиента:
    check_argument(argc, argv, ::client_sockfd);

    // выводим инструктирующий текст для работы с клиентским модулем
    print_from_file(WELCOME_FILE);

    // основной рабочий цикл
    while (true) {
        try {
            fd_set readfds; // множество читаемых дескрипторов
            int maxfds = (is_connect() ? ::client_sockfd : STDIN_FILENO) + 1;
            // печатаем приглашение к вводу
            invite();

            FD_ZERO(&readfds);              // очищаем множество
            if (is_connect()) {      // добавляем во множество дескриптор сокета клиента 
                FD_SET(::client_sockfd, &readfds);
            }
            FD_SET(STDIN_FILENO, &readfds); // добавляем во множество дескриптор стандартного потока ввода
            if (select(maxfds, &readfds, nullptr, nullptr, nullptr) == -1) {
                throw FatalError("client", "select", strerror(errno));
            }

            // введена команда клиентом
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                exec_command(::command, ::client_sockfd);
            }
            // пришло сообщение с сервера
            if (is_connect() && FD_ISSET(::client_sockfd, &readfds)) {
                recv_msg(::client_sockfd);
            }
        } // end try {}
        catch (std::exception &err) {
            std::cerr << err.what() << std::endl; // выводим сообщение об ошибке
            ::command.clear(); // отчищаем буфер команды
            if (isfatal()) {
                if (is_connect()) {
                    // закрываем сокет
                    sock_off(::client_sockfd, is_connect());
                }
                return -1;     // завершаем программу с кодом 255 (-1)
            }
        }
    } // end while(true)
}


void handler(int signum)
{
    // выводим сообщение о приходе сигнала
    std::cerr << std::endl << (signum == SIGINT ? "SIGINT" : "SIGTSTP") << " signal came" << std::endl;
    if (is_connect()) { // если установлено соединение с сервером => разорвать, уведомив сервер
        sock_off(::client_sockfd, is_connect());
    }
    ::command.clear(); // отчищщаем буфер команды
    _exit(signum);     // завершаем программу с кодом signum
}