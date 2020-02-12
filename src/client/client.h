#ifndef SQL_INTERPRETER_CLIENT_H
#define SQL_INTERPRETER_CLIENT_H

#include <string> // std::string


/**
 * [check_argument: checks command line arguments and the ability to immediately connect to the server]
 */
void check_argument(int argc, char *argv[], int &sockfd);


/**
 * [is_connect: returns true if connection is establisged; false - otherwise]
 */
bool is_connect();


/**
 * [connect: creates a socket and connects to the server with <server_IP> and <server_port>]
 */
void connect(int &sockfd, const char *server_IP, int server_port);


/**
 * [disconnect: disconnects from the server]
 */
void disconnect(int &sockfd);


/**
 * [exec_command: executes the user's command or sends a request to the server]
 */
void exec_command(std::string &cmd, int &sockfd);


/* optional parameter read command */
typedef enum
{
    EMPTY, COMMAND, QUERY, INCOMPLETE
} read_option;

/**
 * [read_command: reads the user's command to buffer <cmd>]
 */
void read_command(std::string &cmd, read_option &opt);


/**
 * [recvmsg: reads and prints a message from the server]
 */
void recv_msg(int &sockfd);


/**
 * [sockoff: socket shutdown: if <notify> == true then send a message to the server about shutting down]
 * [                          disconnect from the server and close the descriptor <sockfd>             ]
 */
void sock_off(int &sockfd, bool notify);


/**
 * [invite: prints the entrance invitation]
 */
void invite();


/**
 * [print_from_file: prints the contents of the file <file_name> on the screnn]
 */
void print_from_file(const char *file_name);

#endif // SQL_INTERPRETER_CLIENT_H
