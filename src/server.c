#include "server.h"

void check_err(int val, char *msg)
{
    if (val == -1)
    {
        printf("%s\n", msg);
    }
}

void init(Http_response *response)
{

    strcpy(response->content_type, "");
    strcpy(response->body, "");
    strcpy(response->file_path, "");
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
    char file_path_with_dir[1024] = "../public/";
    strcat(file_path_with_dir, file_path);
    fp = fopen(file_path_with_dir, "r");

    if (fp == NULL)
    {
        printf("Error opening file\n");
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
    char file_path_with_dir[1024] = "../public/";
    strcat(file_path_with_dir, file_path);
    fp = fopen(file_path_with_dir, "w");

    if (fp == NULL)
    {
        printf("Error opening file\n");
        return -1;
    }

    fputs(data, fp);
    fclose(fp);

    return 0;
}

/**
 * @brief Save a JSON object to a file
 *
 * This function takes a file path and a JSON string as input and appends the
 * JSON object to an existing or new JSON array in the file. It uses the cJSON
 * library to parse and print JSON data.
 *
 * @param[in] file_path The relative path of the file to save the JSON object to
 * @param[in] data The JSON string to be saved
 * @return 0 if the operation was successful, -1 otherwise
 */
int save_json(char *file_path, const char *data)
{
    FILE *fp;
    char file_path_with_dir[1024] = "../public/";
    strcat(file_path_with_dir, file_path);
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
        char buffer[MAX_BODY_SIZE];
        size_t bytes_read = fread(buffer, 1, MAX_BODY_SIZE, fp);
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
        printf("Error opening file\n");
        return -1;
    }
    char *json_string = cJSON_Print(json);
    fputs(json_string, fp);
    free(json_string);
    cJSON_Delete(json);
    fclose(fp);

    return 0;
}

/**
 * @brief Sends an HTTP response to a client.
 *
 * This function builds an HTTP response header and sends it to the client,
 * followed by the contents of a file. It uses the TCP_CORK option to minimize
 * the number of packets sent and to tune performance. This is useful when
 * sending a small amount of header data in front of the file contents. - For
 * more see NOTES section for sendfile(2).
 *
 * @param connfd The file descriptor of the connection to the client.
 * @param response The Http_response struct containing the details of the response.
 *
 * @return void
 */
void send_response(int connfd, Http_response response)
{
    // Enable TCP_CORK
    int cork = 1;
    setsockopt(connfd, IPPROTO_TCP, TCP_CORK, &cork, sizeof(cork));

    // open the file
    int fd = open(response.file_path, O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening file\n");
        strcpy(response.status_code, "404 File Not Found");
        strcpy(response.content_type, "text/html");

        // Try to open the 404.html file
        fd = open("../public/404.html", O_RDONLY);
        if (fd == -1)
        {
            printf("Error opening 404.html file\n");
            return;
        }
    }
    else
    {
        strcpy(response.status_code, "200 OK");
    }

    // build response header
    char http_header[MAX_HEADER_SIZE] = "HTTP/1.1 ";
    strcat(http_header, response.status_code);
    strcat(http_header, "\r\n");
    strcat(http_header, "Content-Type: ");
    strcat(http_header, response.content_type);
    strcat(http_header, "\r\n\n");

    // send the response header to the client
    send(connfd, http_header, strlen(http_header), 0);

    // get the size of the file
    struct stat stat_buf;
    if (fstat(fd, &stat_buf) == -1)
    {
        printf("Error getting file size\n");
        return;
    }

    // send the file
    ssize_t sent_bytes = sendfile(connfd, fd, NULL, stat_buf.st_size);
    if (sent_bytes == -1)
    {
        printf("Error sending file\n");
        return;
    }

    // close the file
    close(fd);

    // Disable TCP_CORK
    cork = 0;
    setsockopt(connfd, IPPROTO_TCP, TCP_CORK, &cork, sizeof(cork));

    return;
}

int http_get_hndlr(int connfd, Http_request req)
{
    Http_response new_response;

    if (strcmp(req.file_path, "") == 0)
    {
        printf("tinyserver: file path is empty\n");
        return -1;
    }

    printf("GOT THIS FAR\n");

    char *mime_type = get_mime_type(req.file_path);
    char file_path_with_dir[4096] = "../public/";

    init(&new_response);

    strcpy(new_response.body, req.body);
    strcpy(new_response.content_type, mime_type);
    strcat(file_path_with_dir, req.file_path);
    strcpy(new_response.file_path, file_path_with_dir);

    send_response(connfd, new_response);

    return 0;
}

int http_post_hndlr(int connfd, Http_request req)
{
    if (strcmp(req.body, "") == 0)
    {
        printf("tinyserver: body is empty\n");
        return -1;
    }
    if (strcmp(req.file_path, "") == 0)
    {
        printf("tinyserver: file path is empty\n");
        return -1;
    }

    char *mime_type = get_mime_type(req.file_path);

    if (strcmp(mime_type, "application/json") == 0)
    {
        save_json(req.file_path, req.body);
    }
    else
    {
        save_file(req.file_path, req.body);
    }

    // send simple OK
    send(connfd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);

    return 0;
}

Http_request parse_Http_request(char *request)
{
    Http_request req;
    regex_t regex;
    regmatch_t pmatch[2]; // only need 1 match for file path
    int match;

    // first we need to determine the method (GET or POST)
    match = regcomp(&regex, "^GET", 0); // ^ means start of string
    match = regexec(&regex, request, 0, NULL, 0);
    if (match == 0)
        req.method = HTTP_GET;
    match = regcomp(&regex, "^POST", 0);
    match = regexec(&regex, request, 0, NULL, 0);
    if (match == 0)
    {
        req.method = HTTP_POST;
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
    else
    {
        strcpy(req.body, "");
        printf("Error parsing body\n");
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
    {
        strcpy(req.file_path, "");
        printf("Error parsing file path\n");
    }

    regfree(&regex);

    return req;
}

int read_request(int connfd, char *buffer, size_t buffer_size)
{
    ssize_t bytes_read = recv(connfd, buffer, buffer_size - 1, 0);
    printf("tinyserver: bytes read: %ld\n", bytes_read);
    if (bytes_read < 0)
    {
        printf("recv failed\n");
        return -1;
    }
    else if (bytes_read == 0)
    {
        printf("tinyserver: client disconnected\n");
        return -1;
    }
    buffer[bytes_read] = '\0';

    return 0;
}

void *handle_client(Http_client *client)
{
    /*DEBUG*/
    printf("\n\033[32m##############################################\n");
    printf("\033[0m");

    printf("tinyserver: THIS IS THE START OF A NEW REQUEST\n");
    pthread_t thread_id = pthread_self();
    printf("tinyserver: \033[33mthread id: %ld\033[0m\n", thread_id);

    // read request from client
    char buffer[4096];
    if (read_request(client->connfd, buffer, sizeof(buffer)) == FAIL)
    {
        printf("tinyserver: reading request failed\n");
        // close(client->connfd);
        // free(client);
        return NULL;
    }
    /*DEBUG*/
    printf("tinyserver: buffer contents are:\n\033[36m%s\n\033[0m", buffer);

    client->request = parse_Http_request(buffer);

    // print everything in the request
    printf("tinyserver: method is %d\n", client->request.method);
    printf("tinyserver: file path is %s\n", client->request.file_path);
    printf("tinyserver: body is %s\n", client->request.body);

    switch (client->request.method)
    {
    case HTTP_GET:
        printf("tinyserver: method is GET\n");
        check_err(http_get_hndlr(client->connfd, client->request), "Error handling GET request");
        break;
    case HTTP_POST:
        printf("tinyserver: method is POST\n");
        check_err(http_post_hndlr(client->connfd, client->request), "Error handling POST request");
        break;
    case HTTP_PUT:
        printf("tinyserver: method is PUT\n");
        break;
    default:
        printf("tinyserver: method is unknown\n");
        break;
    }

    close(client->connfd);
    // free(client);

    printf("tinyserver: THIS IS THE END OF A NEW REQUEST\n");
    printf("\033[32m##############################################\n\n\n");
    printf("\033[0m");

    return 0;
}

// void *handle_client_wrapper(void *arg)
// {
//     Http_client *client = (Http_client *)arg;
//     handle_client(client);

//     return NULL;
// }

long double get_cpu_usage()
{
    FILE *fp;
    long double a[4], b[4], loadavg;

    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
    fclose(fp);
    sleep(1);

    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
    fclose(fp);

    loadavg = ((b[0] + b[1] + b[2]) - (a[0] + a[1] + a[2])) / ((b[0] + b[1] + b[2] + b[3]) - (a[0] + a[1] + a[2] + a[3]));
    loadavg *= 100;

    return loadavg;
}

long get_memory_usage()
{
    FILE *fp;
    char buf[BUF_SIZE];
    long mem_usage;

    /* Open the /proc/self/status file. */
    fp = fopen("/proc/self/status", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/self/status");
        return -1;
    }

    /* Read the entire contents. */
    while (fgets(buf, BUF_SIZE, fp) != NULL)
    {
        /* The VmRSS value contains the resident set size, i.e., the portion of the process's memory that is held in RAM. */
        if (strncmp(buf, "VmRSS:", 6) == 0)
        {
            sscanf(buf, "%*s %ld", &mem_usage);
            break;
        }
    }

    fclose(fp);

    return mem_usage; /* kB */
}

ThreadInfo thread_list[MAX_THREADS];
int thread_count = 0;

void *handle_client_wrapper(void *arg)
{
    /* Your existing code here... */

    Http_client *client = (Http_client *)arg;
    handle_client(client);
    /* Update the thread status when finished. */
    for (int i = 0; i < thread_count; i++)
    {
        if (pthread_equal(pthread_self(), thread_list[i].thread_id))
        {
            thread_list[i].status = 1;
            break;
        }
    }

    return NULL;
}

void printUsage(long mem_usage, long double cpu_usage)
{
    if (cpu_usage < 30)
        printf("\033[32m"); // Set the text color to green
    else if (cpu_usage < 60)
        printf("\033[33m"); // Set the text color to yellow
    else
        printf("\033[31m"); // Set the text color to red

    printf("CPU Usage: %.2Lf%%", cpu_usage);

    printf("\033[0m"); // Reset the text color to default

    if (mem_usage < 30)
        printf("\033[32m"); // Set the text color to green
    else if (mem_usage < 60)
        printf("\033[33m"); // Set the text color to yellow
    else
        printf("\033[31m"); // Set the text color to red

    printf(", Memory Usage: %ld kB", mem_usage);

    printf("\033[0m"); // Reset the text color to default
}

int main(int argc, char *argv[])
{
    Http_server server;
    int connection_count = 0;
    strcpy(server.config.root_dir, argv[2]);

    check_err((server.sockfd = socket(AF_INET, SOCK_STREAM, 0)), "Socket error");

    server.server_addr.sin_family = AF_INET;
    server.server_addr.sin_addr.s_addr = INADDR_ANY;
    server.server_addr.sin_port = htons(PORT);

    int optval = 1;
    check_err(setsockopt(server.sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)), "Setsockopt error");

    check_err(bind(server.sockfd, (SA *)&server.server_addr, sizeof(server.server_addr)), "Bind error");
    check_err(listen(server.sockfd, BACKLOG), "Listen error");

    printf("Server: waiting for connection on port %d...\n", ntohs(server.server_addr.sin_port));
    printf("You can access it at: \033[32m\033[4mhttp://10.65.255.109:%d/index.html\033[0m\n", ntohs(server.server_addr.sin_port));

    while (1)
    {
        long double cpu_usage = get_cpu_usage();
        long mem_usage = get_memory_usage();

        Http_client *client = malloc(sizeof(Http_client));
        socklen_t addr_size = sizeof(client->client_addr);

        check_err((client->connfd = accept(server.sockfd, (SA *)&client->client_addr, &addr_size)), "Accept error");

        printf("Server: got connection from %s\n", inet_ntoa(client->client_addr.sin_addr));
        connection_count++;
        printf("Server: connection count is %d\n", connection_count);

        ThreadInfo *info = &thread_list[thread_count++];
        pthread_create(&info->thread_id, NULL, handle_client_wrapper, (void *)client);
        info->status = 0;
        pthread_detach(info->thread_id);

        // printf("\rCPU Usage: %.2Lf%%, Memory Usage: %ld kB", cpu_usage, mem_usage);
        printUsage(mem_usage, cpu_usage);

        fflush(stdout);

        printf("\nThreads:\n");
        for (int i = 0; i < thread_count; i++)
        {
            printf("Thread %d:%ld %s\n", i, thread_list[i].thread_id, thread_list[i].status ? "Finished" : "Running");
        }
    }

    close(server.sockfd);

    return 0;
}