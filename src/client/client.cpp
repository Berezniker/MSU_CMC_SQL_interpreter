#include <iostream>     // std::cout: put(), flush(), std::cin: get(), putback(), std::cerr, std::endl
#include <string>       // std::string: empty(), c_str(), push_back(), clear()
#include <sstream>      // std::istringstream
#include <fstream>      // std::ifstream: is_open(), eof(), close()
#include <stdlib.h>     // strtol(), system()
#include <string.h>     // strcmp(), strncmp(), strerror(), memset()
#include <ctype.h>      // isspace()
#include <errno.h>      // errno
#include <unistd.h>     // _exit(), close()
#include <sys/socket.h> // socket(), connect(), shutdown(), send(), recv()
#include <netinet/in.h> // htons()
#include <arpa/inet.h>  // inet_aton()
#include <exeption/exception.h>  // FatalError, FlexibleError, what(), isfatal()

#include "client.h"     // прототипы всех функций, описанных в этом файле


#define MSG_LEN      1024        // длина отправляемого/получаемого сообщения
#define DEFAULT_IP   "127.0.1.1" // значение IP сервера по умолчанию (для локальной сети)
#define DEFAULT_PORT 5000        // значение порта сервера по умолчанию
#define HELP_FILE  _cmake_assistive_dir"/help_info.txt" // имя файла, содержашего help-инструкцию
#define SQL_BNF_FILE _cmake_assistive_dir"/SQL_BNF.txt" // имя файла, содержащего BNF языка SQL++


bool connect_establish = false;  // флаг = {true, если подключены к серверу; false, иначе}


inline bool is_connect()
{
    return ::connect_establish;
}


void check_argument(int argc, char *argv[], int &sockfd)
{
    if (argc == 2 || argc > 3) {
        // некорректный запуск клиентского модуля с параметрами в командной строке
        std::cout << "Use " << argv[0] << " { [server-IP] [server-port] }" << std::endl;
        _exit(0); // завершаем программу с кодом 0
    }
    else if (argc == 3) {
        // подключаемся к серверу с заданными в командной строке IP и PORT
        try {
            connect(sockfd, argv[1], strtol(argv[2], nullptr, 10));
        }
        catch (std::exception &err) {
            std::cerr << err.what() << std::endl; // выводим сообщение об ошибке
            if (isfatal()) {
                if (sockfd != -1) {
                    // закрываем сокет
                    sock_off(sockfd, is_connect());
                }
                _exit(-1); // завершаем программу с кодом 255 (-1)
            }
        }
    }
    // else argc == 1 - запуск клиен-модуля без парамтеров
}


void connect(int &sockfd, const char *server_IP, int server_port)
{
    if (is_connect()) {
        // не даём повторно подключиться, если соединение уже установлено
        std::cout << "###You are already connected to the server" << std::endl;
        return;
    }
    // создание сокета
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw FatalError("client", "socket", strerror(errno));
    }
    // структура сервера
    struct sockaddr_in server {};
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    if (inet_aton(server_IP, &server.sin_addr) == 0) {
        throw FlexibleError("client", "inet_aton", strerror(errno));
    }
    // установление соединения
    if (connect(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        throw FlexibleError("client", "connect", strerror(errno));
    }
    ::connect_establish = true;
    std::cout << "***Connection established" << std::endl;
}


void disconnect(int &sockfd)
{
    if (!is_connect()) {
        std::cout << "###You are not connected to the server. Try \\connect" << std::endl;
        return;
    }
    // уведомляем сервер о завершении работы
    char end_msg[] = "\\exit";
    if (send(sockfd, end_msg, sizeof(end_msg), 0) == -1) {
        throw FlexibleError("client", "send", strerror(errno));
    }
    // отсоединяемся от сервера
    if (shutdown(sockfd, SHUT_RDWR) == -1) {
        throw FlexibleError("client", "shutdown", strerror(errno));
    }
    // закрываем дескриптор сокета
    if (close(sockfd) == -1) {
        throw FlexibleError("client", "close", strerror(errno));
    }
    sockfd = -1;
    ::connect_establish = false;
    std::cout << "***Disconnected" << std::endl;
}


void exec_command(std::string &cmd, int &sockfd)
{
    read_option opt; // опциональный параметр функции чтения
    // считываем команду
    read_command(cmd, opt);

    if (opt == EMPTY) {
        return; // пустое сообщение (уже обработано)
    }

    if (opt == COMMAND) { // введена команда
        if (cmd == "\\help") {
            print_from_file(HELP_FILE);
        }
        else if (cmd == "\\exit") {
            cmd.clear();  // отчищаем буфер команды
            if (is_connect()) {
                // закрываем сокет
                sock_off(sockfd, is_connect());
            }
            _exit(0);     // завершение программы с кодом 0
        }
        else if (cmd == "\\clear") {
            system("clear");
        }
        else if (cmd == "\\sqlbnf") {
            print_from_file(SQL_BNF_FILE);
        }
        else if (cmd == "\\disconnect") {
            disconnect(sockfd);
        }
        else if (!strncmp(cmd.c_str(), "\\connect", sizeof("\\connect") - 1)) {
            // распарсить аргументы:
            std::string temp, IP, port;
            std::istringstream sin(cmd.c_str() + sizeof("\\connect") - 1);
            sin >> IP;
            sin >> port;
            if (sin >> temp) {       // параметров >= 3
                std::cout << "###Too many arguments" << std::endl;
            }
            else if (IP.empty()) {   // команда без параметров
                connect(sockfd, DEFAULT_IP, DEFAULT_PORT);
            }
            else if (port.empty()) { // 1 параметр => недостаёт 2го
                std::cout << "###Not enough arguments" << std::endl;
            }
            else { // 2 параметра
                connect(sockfd, IP.c_str(), strtol(port.c_str(), nullptr, 10));
            }
        }
        else {
            std::cout << "###Undefined command. Try \\help" << std::endl;
        }
        cmd.clear(); // отчищаем буфер команды
    }
    else if (opt == QUERY) { // введен запрос
        if (is_connect()) {
            // отправляем запрос на сервер
            if (send(sockfd, cmd.c_str(), cmd.length(), 0) == -1) {
                throw FlexibleError("client", "send", strerror(errno));
            }
        }
        else {
            std::cout << "###You are not connected to the server. Try \\connect" << std::endl;
        }
        cmd.clear();               // отчищаем буфер команды
        exec_command(cmd, sockfd); // выполняем следующую команду
    }
    // else (opt == INCOMPLETE) => ждём окончания ввода запроса
}


void read_command(std::string &cmd, read_option &opt)
{
    char c; // текущий символ

    // пропускаем все начальные пробельные символы
    while (isspace(c = std::cin.get()) && c != '\n');

    if (c == '\n') { // пустая строка
        opt = EMPTY; // сообщает на уровень выше что сообщение пустое
        return;
    }
    if (c == '\\' && cmd.empty()) { // команда пользователя
        opt = COMMAND;
    }
    else {
        opt = INCOMPLETE;
    }

    // цикл считывания команды:
    for (int space_flag = 0;
         (opt == COMMAND && c != '\n') || (opt == INCOMPLETE && c != ';' && c != '\n'); c = std::cin.get()) {
        if (isspace(c)) {
            space_flag = 1; // пропускаем повторные пробелы между словами
        }
        else {
            if (space_flag) {
                cmd.push_back(' ');
                space_flag = 0;
            }
            cmd.push_back(c);
        }
    }

    if (c == '\n' && opt != COMMAND) {
        cmd.push_back(' ');
    }
    else if (c == ';') {
        cmd.push_back(c);
        opt = QUERY; // считан SQL запрос
    }
}


void recv_msg(int &sockfd)
{
    char msg_from_server[MSG_LEN + 1] = "\0"; // буфер сообщения
    int nrecv;                                // количество прочитанных символов

    std::cout << "\r    \r";                  // стираем приглашение к вводу
    do {
        // чтение и вывод ответа с сервера
        if ((nrecv = recv(sockfd, msg_from_server, MSG_LEN, 0)) == -1) {
            throw FlexibleError("client", "recv", strerror(errno));
        }
        if (!strcmp(msg_from_server, "\\exit")) { // shutdown server
            std::cout << "***Server shutdown. Disconnect...";
            sock_off(sockfd, false);
            break;
        }
        std::cout << msg_from_server;
    } while (nrecv == MSG_LEN);
    std::cout << std::endl;
}


void sock_off(int &sockfd, bool notify)
{
    if (notify) {
        // уведомляем сервер о завершении работы
        char end_msg[] = "\\exit";
        if (send(sockfd, end_msg, sizeof(end_msg), 0) == -1) {
            std::cerr << "[client] send() " << strerror(errno) << std::endl;
        }
    }
    // отсоединяемся от сервера
    if (shutdown(sockfd, SHUT_RDWR) == -1) {
        std::cerr << "[client] shutdown() " << strerror(errno) << std::endl;
    }
    // закрываем дескриптор сокета
    if (close(sockfd) == -1) {
        std::cerr << "[client] close() " << strerror(errno) << std::endl;
    }
    sockfd = -1;
    ::connect_establish = false;
}


void invite()
{
    if (is_connect()) {
        std::cout << "SQL> ";
    }
    else {
        std::cout << "   > ";
    }
    // выбрасываем все содержимое буфера в консоль
    std::cout.flush();
}


void print_from_file(const char *file_name)
{
    std::ifstream ffile(file_name);    // открываем файл

    if (!ffile.is_open()) {
        throw FlexibleError("client", file_name, strerror(errno));
    }

    while (!ffile.eof()) {             // пока не конец файла
        std::string str;               // str -- строка для чтения
        std::getline(ffile, str);      // считываем очередную строку из файла
        std::cout << str << std::endl; // и выдаём её на дисплей
    }
    ffile.close();                     // закрываем файл
}
