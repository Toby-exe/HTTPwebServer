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

#include <netinet/in.h>
#include <stdbool.h>
#include <sys/resource.h>

#define MAXLINE 4096
#define MAX_BODY_SIZE 1000000
#define MAX_HEADER_SIZE 100000
#define PORT 7079
#define BACKLOG 1000
#define FAIL -1

#define BUF_SIZE 1024
#define MAX_THREADS 1024

#define METHOD_SZ 8

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


// NEW STUFF **********
typedef struct {
    http_method method;
    char path[128];
    char version[16];
    char buffer[MAX_HEADER_SIZE];
    char host[64];
    char connection[64];
    char body[MAX_BODY_SIZE];
} Http_request_header;

typedef struct {
    char *status_code;
    char *content_type;
    char *connection;
    char *status_message;
    char *additional_headers;
} Http_response_header;
////***************

typedef enum {
    OFF,
    ON
} Switch_t;

typedef struct {
    Switch_t enable_mt;
    int num_threads;
    char root_dir[128];
    char port[8];
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

typedef struct {
    pthread_t thread_id;
    int status;  /* 0 = running, 1 = finished */
} ThreadInfo;


#endif /* MY_SHELL_H */