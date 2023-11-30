#include "server.h"

void check_err(int val, char *msg)
{
    if (val == -1)
    {
        perror(msg);
        exit(1);
    }
}

void init(http_response *response)
{
    strcpy(response->status_code, "");
    strcpy(response->content_type, "");
    strcpy(response->body, "");
}

char *get_mime_type(char *type)
{
    // remove everything up to and including '.' from file name
    type = strrchr(type, '.');
    type++;

    if (strcmp(type, "html") == 0)
    {
        return "text/html";
    }
    else if (strcmp(type, "css") == 0)
    {
        return "text/css";
    }
    else if (strcmp(type, "js") == 0)
    {
        return "application/javascript";
    }
    else if (strcmp(type, "jpg") == 0)
    {
        return "image/jpeg";
    }
    else if (strcmp(type, "png") == 0)
    {
        return "image/png";
    }
    else if (strcmp(type, "gif") == 0)
    {
        return "image/gif";
    }
    else if (strcmp(type, "ico") == 0)
    {
        return "image/x-icon";
    }
    else if (strcmp(type, "txt") == 0)
    {
        return "text/plain";
    }
    else if (strcmp(type, "json") == 0)
    {
        return "application/json";
    }
    else
    {
        return "text/plain";
    }
}

char *get_file(char *file_path)
{
    // get file from disk
    FILE *fp;
    char file_path_with_dir[1024] = "public/";
    strcat(file_path_with_dir, file_path);
    fp = fopen(file_path_with_dir, "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    static char file_data[MAX_BODY_SIZE] = "";

    memset(file_data, 0, sizeof(file_data)); // this is to stop the file_data from appending to itself

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strlen(file_data) + strlen(line) + 1 > sizeof(file_data))
        {
            fprintf(stderr, "File buffer size exceeded\n");
            break;
        }
        strcat(file_data, line);
    }

    fclose(fp);

    return file_data;
}

int save_file(char *file_path, const char *data)
{   
    FILE *fp;
    char file_path_with_dir[1024] = "public/";
    strcat(file_path_with_dir, file_path);
    char *mime_type = get_mime_type(file_path);
    fp = fopen(file_path_with_dir, "r");
    cJSON *json;

    if (fp == NULL)
    {
        // if the file doesn't exist, create a new json array
        json = cJSON_CreateArray();
    }
    else
    {
        // if the file exists, read its content
        char buffer[4096];
        size_t bytes_read = fread(buffer, 1, 4096, fp);
        fclose(fp);

        if (bytes_read == 0)
        {
            // if the file is empty, create a new json array
            json = cJSON_CreateArray();
        }
        else
        {
            // if the file is not empty, parse the existing json array
            json = cJSON_Parse(buffer);
        }
    }

    // create a new json object from the request body
    cJSON *new_object = cJSON_Parse(data);

    // add the new object to the json array
    cJSON_AddItemToArray(json, new_object);

    printf("tinyserver: json is %s\n", cJSON_Print(json));

    // write the updated json array back to the file
    fp = fopen(file_path_with_dir, "w");
    if (fp == NULL)
    {
        perror("Error opening file");
        return -1;
    }
    char *json_string = cJSON_Print(json);
    fputs(json_string, fp);
    free(json_string);
    cJSON_Delete(json);
    fclose(fp);

    return 0;
}

void send_response(int connfd, http_response response)
{
    // build response header
    char http_header[1024] = "HTTP/1.1 ";
    strcat(http_header, response.status_code);
    strcat(http_header, "\r\n");
    strcat(http_header, "Content-Type: ");
    strcat(http_header, response.content_type);
    strcat(http_header, "\r\n\n");

    // send the response header to the client
    send(connfd, http_header, strlen(http_header), 0);

    // open the file
    int fd = open(response.file_path, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        return;
    }
    
    // get the size of the file
    struct stat stat_buf;
    if (fstat(fd, &stat_buf) == -1)
    {
        perror("Error getting file size");
        return;
    }

    // send the file
    ssize_t sent_bytes = sendfile(connfd, fd, NULL, stat_buf.st_size);
    if (sent_bytes == -1)
    {
        perror("Error sending file");
        return;
    }

    // close the file
    close(fd);

    return;
}

void response_fnf(int connfd)
{
    // send 404 File Not Found response to client use send_response()
    http_response new_response;
    char *response_data = get_file("404.html");

    init(&new_response);

    strcpy(new_response.status_code, "404 File Not Found");
    strcpy(new_response.body, response_data);
    strcpy(new_response.content_type, "text/html");

    send_response(connfd, new_response);

    return;
}

void response_ok(int connfd, request_data req_data, char *res_data)
{

    // send 200 OK response to client and the requested file use send_response()
    // this needs to be modfied to send the correct content type (mime type)
    // ie we need to match the file extension to the correct mime type
    http_response new_response;

    init(&new_response);

    strcpy(new_response.status_code, "200 OK");
    strcpy(new_response.body, res_data);
    // get mime type based on file extension
    // for now we will just string compare to type but this is O(n)
    char *mime_type = get_mime_type(req_data.file_path);

    printf("tinyserver: mime type is %s\n", mime_type);

    strcpy(new_response.content_type, mime_type);

    // append public/ to the file path
    char file_path_with_dir[1024] = "public/";
    strcat(file_path_with_dir, req_data.file_path);
    strcpy(new_response.file_path, file_path_with_dir);

    send_response(connfd, new_response);

    return;
}

void http_get_hndlr(int connfd, request_data req_data)
{
    printf("tinyserver: getting file %s\n", req_data.file_path);

    char *file_data = get_file(req_data.file_path);

    if (file_data == NULL) // file not found (404)
    {
        response_fnf(connfd);
    }
    else // file found (200)
    {
        response_ok(connfd, req_data, file_data);
    }

    return;
}

request_data parse_http_request(const char *request)
{
    request_data req;
    regex_t regex;
    regmatch_t pmatch[2]; // only need 1 match for file path
    int match;

    // first we need to determine the method (GET or POST)
    match = regcomp(&regex, "^GET", 0); // ^ means start of string
    match = regexec(&regex, request, 0, NULL, 0);
    if (match == 0)
        strcpy(req.method, "GET");

    match = regcomp(&regex, "^POST", 0);
    match = regexec(&regex, request, 0, NULL, 0);
    if (match == 0)
    {
        strcpy(req.method, "POST");
        // right after the two new lines is the body of the request
        char *body_start = strstr(request, "\r\n\r\n");
        if (body_start != NULL)
        {
            body_start += 4; // skip past the "\r\n\r\n"
            char *body = strdup(body_start);
            // now `body` contains the body of the request
            strcpy(req.body, body);
        }
    }

    // next we need to determine the file path
    match = regcomp(&regex, " (/[^ ]*)", REG_EXTENDED);
    match = regexec(&regex, request, 2, pmatch, 0);
    pmatch[0].rm_so = 0;
    if (match == 0)
    {
        int start = pmatch[1].rm_so;
        int end = pmatch[1].rm_eo;
        strncpy(req.file_path, &request[start + 1], end - start - 1); // +1 to skip the first '/' character
        req.file_path[end - start - 1] = '\0';                        // null terminate the string
    }
    else
        perror("Error parsing file path");

    regfree(&regex);

    return req;
}

void *handle_client(http_request *request)
{
    // get thread id
    pthread_t thread_id = pthread_self();
    // priont in yellow
    printf("\033[33m");
    printf("tinyserver: thread id is %ld\n", thread_id);
    // print in default color
    printf("\033[0m");
    
    // read request from client
    char buffer[4096];
    ssize_t bytes_read = recv(request->connfd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read < 0)
    {
        perror("recv failed");
        return NULL;
    }

    // Null-terminate the received data
    buffer[bytes_read] = '\0';

    printf("tinyserver: got request from client\n");
    printf("This is the buffer contents\n%s\n", buffer);

    // parse request
    request->data = parse_http_request(buffer);

    printf("tinyserver: file path is %s\n", request->data.file_path);
    printf("tinyserver: body is %s\n", request->data.body);

    if (strcmp(request->data.method, "GET") == 0)
    {
        printf("tinyserver: got GET request\n");
        http_get_hndlr(request->connfd, request->data);
    }
    else if (strcmp(request->data.method, "POST") == 0)
    {
        printf("tinyserver: got POST request\n");

        if (request->data.body == NULL)
        {
            printf("tinyserver: body is empty\n");
            return NULL;
        }
        // open the file in read mode to check if it exists and is not empty
        save_file(request->data.file_path, request->data.body);
        // send simple OK
        send(request->connfd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);        
    }

    else if (strcmp(request->data.method, "PUT") == 0)
    {
        printf("tinyserver: got PUT request\n");
        // save_file(connfd, request.file_path);
    }
    else
    {
        printf("tinyserver: got unknown request\n");
    }

    close(request->connfd);
    free(request);

    return NULL;
}

void *handle_client_wrapper(void *arg)
{
    http_request *request = (http_request *)arg;
    handle_client(request);
    return NULL;
}

int main(int argc, char *argv[])
{
    server tinyserver;

    check_err((tinyserver.sockfd = socket(AF_INET, SOCK_STREAM, 0)), "Socket error");

    tinyserver.servaddr.sin_family = AF_INET;
    tinyserver.servaddr.sin_addr.s_addr = INADDR_ANY;
    tinyserver.servaddr.sin_port = htons(PORT);

    int optval = 1;
    check_err(setsockopt(tinyserver.sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)), "Setsockopt error");

    check_err(bind(tinyserver.sockfd, (SA *)&tinyserver.servaddr, sizeof(tinyserver.servaddr)), "Bind error");
    check_err(listen(tinyserver.sockfd, BACKLOG), "Listen error");

    // allow the same port to be used again
    printf("tinyserver: waiting for connection on port %d...\n", ntohs(tinyserver.servaddr.sin_port));
    printf("You can access it at: \033[32m\033[4mhttp://10.65.255.109:%d/index.html\033[0m\n", ntohs(tinyserver.servaddr.sin_port));

    while (1)
    {
        // tinyserver.addr_size = sizeof(SA_IN);
        socklen_t addr_size = sizeof(tinyserver.cliaddr);

        http_request *client_req = malloc(sizeof(http_request));

        // check_err((tinyserver.http_req.connfd = accept(tinyserver.sockfd, (SA *)&tinyserver.cliaddr, &tinyserver.addr_size)), "Accept error");
        check_err((client_req->connfd = accept(tinyserver.sockfd, (SA *)&tinyserver.cliaddr, &addr_size)), "Accept error");

        printf("tinyserver: got connection from %s\n", inet_ntoa(tinyserver.cliaddr.sin_addr));

        // handle request
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client_wrapper, (void *)client_req);
        pthread_detach(thread_id);

        // handle_client(tinyserver.http_req);
        // close(tinyserver.connfd);
    }

    close(tinyserver.sockfd);

    return 0;
}