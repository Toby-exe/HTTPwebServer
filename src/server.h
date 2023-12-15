/**
 * @file server.h
 * @brief A library for an simple HTTP web server 
 * @authors Tobias Wondwossen, Jayden Mingle
 * 
 * Details: 
 * - This library provides the necessary data structures and function prototypes for an HTTP web server program. It 
 *   includes the definitions of various structures such as Http_client, Http_request_header, Http_response_header, 
 *   Server_config, Thread_args, and Http_server.
 * - Function prototypes for handling HTTP requests and responses, managing server configuration, handling multi-threading, 
 *   serving files and directories, and handling client connections are provided.
 * 
 * Structures:
 * - Http_client: Represents a client connection with its connection file descriptor and client address.
 * - Http_request_header: Represents an HTTP request header with its method, path, version, host, connection, buffer, and body.
 * - Http_response_header: Represents an HTTP response header with its status code, content type, connection, status message, 
 *   and additional headers.
 * - Server_config: Contains flags for multi-threading, number of threads, root directory, and port.
 * - Thread_args: Contains a pointer to an Http_client and a Server_config.
 * - Http_server: Represents the server with its socket file descriptor, server address, and server configuration.
 * 
 * Function Prototypes:
 * - Handling HTTP requests and responses
 * - Managing server configuration
 * - Handling multi-threading
 * - Processing HTTP request headers
 * - Processing HTTP response headers
 * - Serving files and directories
 * - Handling client connections
 * - Starting the server
 * - Accepting client connections
 * - Printing the server logo
 * 
 * Assumptions/Limitations: 
 * - This library assumes that the maximum size of various elements in the HTTP request and response headers are 
 *   defined by constants such as MAX_PATH_SIZE, MAX_VERSION_SIZE, MAX_HEADER_SIZE, MAX_HOST_SIZE, MAX_CONNECTION_SIZE, 
 *   MAX_BODY_SIZE, MAX_STATUS_CODE_SIZE, MAX_CONTENT_TYPE_SIZE, MAX_STATUS_MESSAGE_SIZE, MAX_ADDITIONAL_HEADERS_SIZE, 
 *   and MAX_ROOT_DIR_SIZE.
 * - It also assumes that the read end of a pipe is 0 and the write end is 1.
 * - It does not handle cases where these limits are exceeded.
 * - For commands such as GET, POST, the request must be properly formatted according to the HTTP/1.1 standard.
 * - The server supports keep-alive connections.
 * - The server supports multi-threading.
 * 
 * Notes:
 * - Due to the nature of supporting "keep-alive" connections, performance if the user chooses < 2 threads 
 *   is suboptimal. This is because the server will wait for the client to close the connection before 
 *   accepting another connection. This is not an issue if the user chooses >= 2 threads.
 *
 * Valid Command: 
 * `GET /path HTTP/1.1\r\nHost: hostname\r\nConnection: keep-alive\r\n\r\n`
 *
 * This server does not have exhaustive error handling. In the event of a misused command or invalid input that is one of the many 
 * that are not handled. In most cases it will simply respond with a 404 error.
 * 
 * @date 2023-12-06
 */

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string.h>
#include <regex.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <netinet/tcp.h>
#include <sys/times.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/resource.h>
#include "pool.h"
#include "mime.h"
#include "files.h"
#include "stats.h"

#define DEFAULT_PORT "8080"
#define DEFAULT_ROOT_DIR "../public"
#define DIR_LISTING_PAGE_SZ ((1024 * 1024) + 4096)
#define DEFAULT_NUM_THREADS 8

#define MAX_BODY_SIZE 1000000
#define MAX_HEADER_SIZE 100000
#define MAX_PATH_SIZE 256
#define MAX_VERSION_SIZE 16
#define MAX_HOST_SIZE 64
#define MAX_CONNECTION_SIZE 64
#define MAX_STATUS_CODE_SIZE 4
#define MAX_CONTENT_TYPE_SIZE 64
#define MAX_STATUS_MESSAGE_SIZE 64
#define MAX_ADDITIONAL_HEADERS_SIZE 1024
#define MAX_ROOT_DIR_SIZE 128
#define MAX_PORT_SIZE 8
#define MAXLINE 4096
#define BACKLOG 1000
#define FAIL -1
#define BUF_SIZE 1024
#define MAX_THREADS 128

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

typedef enum {
    HTTP_GET,
    HTTP_POST,
    // Add more methods here as needed
} http_method;

typedef struct {
    int connfd;
    SA_IN client_addr;
} Http_client;

typedef enum {
    OFF,
    ON
} Switch_t;

typedef struct {
    http_method method;
    char path[MAX_PATH_SIZE];
    char version[MAX_VERSION_SIZE];
    char buffer[MAX_HEADER_SIZE];
    char host[MAX_HOST_SIZE];
    char connection[MAX_CONNECTION_SIZE];
    char body[MAX_BODY_SIZE];
} Http_request_header;

typedef struct {
    char status_code[MAX_STATUS_CODE_SIZE];
    char content_type[MAX_CONTENT_TYPE_SIZE];
    char connection[MAX_CONNECTION_SIZE];
    char status_message[MAX_STATUS_MESSAGE_SIZE];
    char additional_headers[MAX_ADDITIONAL_HEADERS_SIZE];
} Http_response_header;

typedef struct {
    Switch_t enable_stats;
    Switch_t enable_mt;
    Switch_t enable_keep_alive;
    int num_threads;
    char root_dir[MAX_ROOT_DIR_SIZE];
    char port[MAX_PORT_SIZE];
} Server_config;

typedef struct {
    Http_client *client;
    Server_config *server_config;
} Thread_args;

typedef struct {
    int sockfd;
    SA_IN server_addr;
    Server_config config;
} Http_server;


void check_err(int val, char *msg);
void parse_field(char *src, char *des, const char *field);
int handle_http_request(const int connfd, Http_request_header *req_header);
void send_response(const int connfd, Http_response_header res_header, long file_size);
void serve_file(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config);
void serve_dir(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config);
void serve_request(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config);
void serve_request_404(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config);
void http_post_handler(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config);
void http_get_handler(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config);
void handle_client(Http_client *client, Server_config server_config);
void handle_client(Http_client *client, Server_config server_config);
void *handle_client_wrapper(void *arg);
void start_server(Http_server *server, int argc, char *argv[]);
void accept_client(Http_server *server, ThreadPool *pool, int *connection_count);
void print_logo();
void calculate_usage(struct timeval start, struct timeval wall_start);


#endif /* MY_SHELL_H */