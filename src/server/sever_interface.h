#ifndef SQL_INTERPRETATOR_USER_INTERFACE_H
#define SQL_INTERPRETATOR_USER_INTERFACE_H

#include <semaphore.h> // sem_t
#include <thread>      // std::thread
#include "server.h"    // Server

/* ------------------------------------------------- */
/* ---------------- USER_INTERFACE ----------------- */
/* ------------------------------------------------- */

class Server_interface
{
public:
    /**
     * [constructor: [server] - link to controllable server,
     * [sem] - link to controllable semaphore]
     */
    Server_interface(Server& server, sem_t& sem);

    /**
     * [destructor: stops the server if necessary]
     */
    ~Server_interface();

    /**
    * [start_listen: start listener]
    */
    void start_listen();

    /**
    * [stop_listen: start listener]
    */
    void stop_listen();

private:
    Server& server;     // ссылка на действующий сервер /* TODO добавить функциональность */
    sem_t& sem;         // семафор, на котором висит main()
    int socket_stop[2]; // канал для остановки треда
    std::thread _listen_thread;
    constexpr static const char stop_message[] = "1"; // сообщение для остановки треда

    /**
    * [on_listen: main loop]
    */
    void on_listen();

    bool is_on_run; // состояние интерфейса
};

#endif // SQL_INTERPRETATOR_USER_INTERFACE_H
