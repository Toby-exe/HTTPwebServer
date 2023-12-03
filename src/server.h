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

#include <netinet/in.h>
#include <stdbool.h>

#define MAXLINE 4096
#define MAX_BODY_SIZE 1000000
#define MAX_HEADER_SIZE 100000
#define PORT 7078
#define BACKLOG 1000
#define FAIL -1

#define BUF_SIZE 1024
#define MAX_THREADS 1024

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT
    // Add more methods here as needed
} http_method;

typedef struct {
    http_method method;
    char url[MAXLINE];
    char body[MAX_BODY_SIZE];
    char file_path[MAXLINE];
} Http_request;

typedef struct {
    char status_code[MAXLINE];
    char content_type[MAXLINE];
    int num_headers;
    char body[MAX_BODY_SIZE];
    char file_path[MAXLINE];
} Http_response;

typedef struct {
    int connfd;
    SA_IN client_addr;
    Http_request request;
    // http_response response;
} Http_client;

typedef enum {
    OFF,
    ON
} Switch_t;

typedef struct {
    Switch_t enable_mt;
    Switch_t enable_cache;
    int num_threads;
    int max_queue_size;
    int max_cache_size;
    char root_dir[MAXLINE];
} Server_config;

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