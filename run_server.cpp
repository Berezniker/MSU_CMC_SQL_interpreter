#include <iostream>         // std::cerr, std::endl
#include <exception>        // std::runtime_error, what()
#include <unistd.h>         // _exit()
#include <signal.h>         // signal(), SIGINT, SIGTSTP
#include <semaphore.h>      // sem_t, sem_init(), sem_wait(), sem_post()
#include <server/server.h> // start(), stop()
#include <server/sever_interface.h>// strat_listen(), stop_listen()

#define PORT   5000         // номер порта сервера
#define QUEUE_LEN 5         // длина очереди в listen()

sem_t stop_semaphore;       // Signal set that to notify application about time to stop

/*
 An integer type which can be accessed as an atomic entity even
 in the presence of asynchronous interrupts made by signals
 */
volatile sig_atomic_t stop_reason = 0; // сигнал, по которому останавливается программа

/**
 * [handler: SIGINT(<Ctrl+C>) and SIGTSTP(<Ctrl+Z>) signals handler]
 */
void handler(int signum)
{
    stop_reason = signum;
    // разблокируем семафор
    sem_post(&stop_semaphore);
}


int main()
{
    try {
        // Using semaphore for communication between main thread AND signal handler
        if (sem_init(&stop_semaphore, 0, 0) != 0) {
            throw std::runtime_error("Failed to create semaphore");
        }
    }
    catch (std::exception &err) {
        // выводим сообщение об ошибке:
        std::cerr << err.what() << std::endl;
        return -1; // завершаем работу сервера с кодом 255 (-1)
    }

    // устанавливаем обработку сигналов <Ctrl+C> и <Ctrl+Z>:
    signal(SIGINT, handler);
    signal(SIGTSTP, handler);

    // создаём сервер и интерфейс
    Server server(PORT, QUEUE_LEN);
    Server_interface interface(server, stop_semaphore);

    try {
        // запускаем сервер
        server.start();
        // запускаем интерфейс
        interface.start_listen();

        sem_wait(&stop_semaphore);

        // сообщаем о приходе сигнала на сервер:
        std::cerr << std::endl << "stop signal: " << stop_reason << std::endl;
        interface.stop_listen(); // отключаем интерфейс
        server.stop();           // отключаем и закрываем сокет
        return stop_reason;      // завершаем работу сервера с кодом stop_reason
    }
    catch (std::exception &err) {
        // выводим сообщение об ошибке:
        std::cerr << err.what() << std::endl;
        interface.stop_listen(); // отключаем интерфейс
        server.stop();           // отключаем и закрываем сокет
        return -1;               // завершаем работу сервера с кодом 255 (-1)
    }
}
