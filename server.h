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

#define MAXLINE 1024
// max body size is 2MB
#define MAX_BODY_SIZE 1000000
#define PORT 7079
#define BACKLOG 1000

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

typedef struct
{
    char content_type[MAXLINE];
    char content_length[MAXLINE];
    char content[MAX_BODY_SIZE];
    char method[MAXLINE];
    char file_path[MAXLINE];
    char body[MAX_BODY_SIZE];
} request_data;

typedef struct
{
    int connfd;
    request_data data;
} http_request;

typedef struct
{
    char content_type[MAXLINE];
    char content_length[MAXLINE];
    char last_modified[MAXLINE];
    char status_code[MAXLINE];
    char body[MAX_BODY_SIZE];
    char file_path[MAXLINE];
} http_response;

typedef struct
{
    bool enable_mt;
    bool enable_cache;
    int num_threads;
    int max_queue_size;
    int max_cache_size;
    char root_dir[MAXLINE];
} server_config;

typedef struct
{
    int sockfd;
    SA_IN servaddr;
    SA_IN cliaddr;
    server_config config;
} server;

// main purpose of this is for the dashboard to customize the server spec

#endif