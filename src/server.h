#ifndef MY_SHELL_H
#define MY_SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "cJSON.h"
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <netinet/tcp.h>
#include <sys/times.h>
#include <sys/time.h>
#include <dirent.h>
#include "pool.h"
#include "mime.h"

#define DEFAULT_PORT "8080"
#define DEFAULT_ROOT_DIR "../public"
#define DIR_LISTING_PAGE_SZ ((1024 * 1024) + 4096)

#define MAX_BODY_SIZE 1000000
#define MAX_HEADER_SIZE 100000
#define MAX_PATH_SIZE 128
#define MAX_VERSION_SIZE 16
#define MAX_HOST_SIZE 64
#define MAX_CONNECTION_SIZE 64
#define MAX_STATUS_CODE_SIZE 4
#define MAX_CONTENT_TYPE_SIZE 64
#define MAX_STATUS_MESSAGE_SIZE 64
#define MAX_ADDITIONAL_HEADERS_SIZE 1024
#define MAX_ROOT_DIR_SIZE 128
#define MAX_PORT_SIZE 8


#include <netinet/in.h>
#include <stdbool.h>
#include <sys/resource.h>

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
    HTTP_PUT
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
    Switch_t enable_mt;
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
int save_json(char *file_path, const char *data);
int save_file(char *file_path, const char *data);
bool file_exists(char *path, char *root_dir);
bool file_exists(char *path, char *root_dir);
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
long get_memory_usage();
double calculate_cpu_usage();
void calculate_usage(struct timeval start, struct timeval wall_start);
void start_server(Http_server *server, int argc, char *argv[]);
void accept_client(Http_server *server, ThreadPool *pool, int *connection_count);
void print_logo();

#endif /* MY_SHELL_H */