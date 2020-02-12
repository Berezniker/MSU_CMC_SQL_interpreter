#ifndef SQL_INTERPRETER_SERVER_H
#define SQL_INTERPRETER_SERVER_H

#include <thread> // std::thread
#include <list>   // std::list<int>

/* ------------------------------------------------- */
/* -------------------- SERVER --------------------- */
/* ------------------------------------------------- */

#define MSG_LEN 1024 // длина получаемого/отправляемого сообщения

class Server
{
public:
    /**
     * [constructor: create server op [port] with max queue length [queue_len] ]
     */
    Server(int port, int queue_len);

    /**
     * [destructor: remove server]
     */
    ~Server();

    /**
     * [start: start server]
     */
    void start();

    /**
     * [stop: stop server]
     */
    void stop();

private:
    const int port;                   // номер порта сервера
    const int queue_len;              // длина очереди в listen()
    int listener = -1;                // дескриптор слушающего сокета
    int socket_stop[2];               // дескриптор для остановки сервера
    std::list<int> client_connections;// база данных дескрипторов подключенных клиентов
    std::thread _work_thread;         // в этом треде обрабатываем соединения
    constexpr static const char stop_message[] = "1"; // сигнал для остановки сервера
    bool is_on_run;                   // состояние сервера

    /**
     * [on_run: main loop]
     */
    void on_run();

    /**
     * [install_server: server and socket settings]
     */
    void install_server();

    /**
     * [accep_tclient: establishment of connection with new client and add it to the <client_connections>]
     */
    void accept_client();

    /**
     * [rec_vmsg_from: receive message from user]
     */
    void recv_msg_from(int user_sockfd);

    /**
     *  [send2all: sends message to all users in <client_connections>]
     */
    void send2all(const char *message1, ...) const;
}; // class Server

#endif // SQL_INTERPRETER_SERVER_H
