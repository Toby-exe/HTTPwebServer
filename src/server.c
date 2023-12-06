#include "server.h"

void check_err(int val, char *msg)
{
    if (val == -1)
    {
        printf("%s\n", msg);
    }
}

int save_file(char *file_path, const char *data)
{
    FILE *fp;
    char file_path_with_dir[1024] = "../public";
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
    char file_path_with_dir[1024] = "../public";
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

bool file_exists(char *path, char *root_dir)
{
    char full_path[BUF_SIZE];
    strcpy(full_path, root_dir);
    strcat(full_path, path);

    printf("checking if file exists at: %s\n", full_path);

    if (access(full_path, F_OK) == -1)
    {
        printf("File does not exist\n");
        return false;
    }
    else
    {
        return true;
    }
}

void parse_field(char *src, char *des, const char *field)
{
    char *start, *end;
    // Find the field in the source string
    start = strstr(src, field);
    if (start == NULL)
    {
        printf("Field not found.\n");
        return;
    }

    // Skip the field and the following colon and space
    start += strlen(field) + 2;

    // Find the end of the field value
    end = strchr(start, '\n');
    if (end == NULL)
    {
        end = src + strlen(src);
    }

    // Copy the field value to the destination string
    strncpy(des, start, end - start);
    des[end - start] = '\0'; // Null terminate the destination string
}

int handle_http_request(const int connfd, Http_request_header *req_header)
{

    memset(req_header, 0, sizeof(Http_request_header));

    ssize_t bytes_read = recv(connfd, req_header->buffer, MAX_HEADER_SIZE, 0);
    if (bytes_read < 0)
    {
        printf("Error reading from socket\n");
        return -1;
    }
    if (bytes_read < 3)
    {
        printf("Invalid request\n");
        return -1;
    }
    req_header->buffer[bytes_read] = '\0';

    printf("Request header:\n%s\n", req_header->buffer);

    // see what the request is (GET, POST, etc)
    regex_t regex;
    regmatch_t pmatch[3]; // only need 1 match for file path
    int match;

    // ^ means start of string
    match = regcomp(&regex, "^GET", 0);
    match = regexec(&regex, req_header->buffer, 2, pmatch, 0);
    if (match == 0)
    {
        req_header->method = HTTP_GET;
    }

    match = regcomp(&regex, "^POST", 0);
    match = regexec(&regex, req_header->buffer, 2, pmatch, 0);
    if (match == 0)
    {
        req_header->method = HTTP_POST;

        char *body_start = strstr(req_header->buffer, "\r\n\r\n");
        if (body_start != NULL)
        {
            body_start += 4; // skip past the "\r\n\r\n"
            char *body = strdup(body_start);
            // now `body` contains the body of the request
            strcpy(req_header->body, body);
        }
        else
        {
            strcpy(req_header->body, " ");
            printf("Error parsing body\n");
        }
    }

    match = regcomp(&regex, "^(GET|POST) ([^ ]*) HTTP", REG_EXTENDED);
    if (match != 0)
    {
        printf("Error compiling regex\n");
        return -1;
    }

    // Execute the regular expression
    match = regexec(&regex, req_header->buffer, 3, pmatch, 0);
    if (match != 0)
    {
        printf("Error executing regex\n");
        return -1;
    }

    // Extract the path from the request line
    size_t path_length = pmatch[2].rm_eo - pmatch[2].rm_so;
    strncpy(req_header->path, req_header->buffer + pmatch[2].rm_so, path_length);
    req_header->path[path_length] = '\0'; // Null-terminate the path

    // Clean up
    regfree(&regex);

    parse_field(req_header->buffer, req_header->host, "Host");

    return 0;
}

void serve_request(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config)
{
    // create state machine either in serve file or server dir
    enum state
    {
        SERVE_FILE,
        SERVE_DIR
    };

    enum state current_state = SERVE_FILE;
    // print thread id in yellow
    char response[MAX_HEADER_SIZE];
    char file_path[256];
    struct stat file_stat;
    int fd;
    long file_size = 0;
    char *page_buffer;

    memset(&res_header.content_type, 0, sizeof(res_header.content_type));
    memset(response, 0, sizeof(response));
    memset(file_path, 0, sizeof(file_path));

    printf("inside serve_request\n");

    // Construct the full path
    char full_path[4096]; // Adjust size as needed
    snprintf(full_path, sizeof(full_path), "%s%s", server_config.root_dir, req_header.path);

    // Get file or directory information
    struct stat path_stat;
    if (stat(full_path, &path_stat) == -1)
    {
        perror("stat");
        return;
    }

    if (S_ISDIR(path_stat.st_mode))
    {
        current_state = SERVE_DIR;
        // Open the directory
        DIR *dir = opendir(full_path);
        if (dir == NULL)
        {
            perror("opendir");
            return;
        }

        // Start the HTML response
        char *html_start = "<html><head><style>"
                           "body {font-family: Arial, sans-serif; margin:0; padding:0; background-color: #f0f0f0;}"
                           "ul {list-style-type: none; margin: 0; padding: 0;}"
                           "li {padding: 10px 0; border-bottom: 1px solid #ddd;}"
                           "li:last-child {border-bottom: none;}"
                           "li a {text-decoration: none; color: #333; display: block; padding: 10px;}"
                           "li a:hover {background-color: #ddd;}"
                           "</style></head><body><ul>";
        char *html_end = "</ul></body></html>";
        char html_body[4096] = ""; // Adjust size as needed

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            // Skip . and .. entries
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            // Add a list item with a link to the file
            char item[1024]; // Adjust size as needed
            if (strcmp(req_header.path, "/") == 0)
            {
                // If we are in the root directory
                snprintf(item, sizeof(item), "<li><a href=\"/%s\">%s</a></li>", entry->d_name, entry->d_name);
            }
            else
            {
                // If we are in a subdirectory
                snprintf(item, sizeof(item), "<li><a href=\"%s/%s\">%s</a></li>", req_header.path, entry->d_name, entry->d_name);
            }
            strncat(html_body, item, sizeof(html_body) - strlen(html_body) - 1);
        }

        // Close the directory
        closedir(dir);

        // Construct the full HTML response
        char html_response[4096]; // Adjust size as needed
        snprintf(html_response, sizeof(html_response), "%s%s%s", html_start, html_body, html_end);

        // Set the response fields
        res_header.content_type = "text/html";
        file_size = strlen(html_response);

        size_t buffer_size = strlen(html_start) + strlen(html_body) + strlen(html_end) + 1;
        page_buffer = malloc(buffer_size);
        if (page_buffer == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for page_buffer.\n");
            return;
        }
        snprintf(page_buffer, buffer_size, "%s%s%s", html_start, html_body, html_end);
    }
    else if (S_ISREG(path_stat.st_mode))
    {
        current_state = SERVE_FILE;
        res_header.content_type = get_mime_type(req_header.path);

        snprintf(file_path, sizeof(file_path), "%s%s", server_config.root_dir, req_header.path);
        printf("Serving file: %s\n", file_path);
        // Open the file
        fd = open(file_path, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            return;
        }

        // Get file stats
        if (fstat(fd, &file_stat) < 0)
        {
            perror("fstat");
            return;
        }

        file_size = file_stat.st_size;
    }

    // Construct the response header
    snprintf(response, sizeof(response),
             "HTTP/1.1 %s %s\r\n"
             "Content-Length: %ld\r\n"
             "Content-Type: %s\r\n"
             "Connection: %s\r\n"
             "%s\r\n",
             res_header.status_code, res_header.status_message,
             file_size,
             res_header.content_type,
             res_header.connection,
             res_header.additional_headers);

    printf("Response header:\n%s\n", response);

    // Send the response header
    if (send(connfd, response, strlen(response), 0) == -1)
    {
        perror("send");
        return;
    }

    if (current_state == SERVE_FILE)
    {
        // Send the file
        if (sendfile(connfd, fd, NULL, file_size) == -1)
        {
            perror("sendfile");
            return;
        }
    }
    else if (current_state == SERVE_DIR)
    {
        printf("sending page: %s\n", page_buffer);
        // Send the file
        if (send(connfd, page_buffer, strlen(page_buffer), 0) == -1)
        {
            perror("send");
            return;
        }
    }

    // Close the file
    close(fd);
}

void handle_client(Http_client *client, Server_config server_config)
{
    printf("\033[33mThread %ld\033[0m\n", pthread_self());

    Http_response_header res_header;
    bool keep_alive = false;
    fd_set set;
    struct timeval timeout;

    memset(&res_header, 0, sizeof(res_header));
    res_header.status_code = "200";
    res_header.status_message = "OK";
    res_header.additional_headers = "Keep-Alive: timeout=10\r\nServer: tinyserver\r\n";
    res_header.connection = "close";

    do
    {
        // Initialize the file descriptor set.
        FD_ZERO(&set);
        FD_SET(client->connfd, &set);

        // Initialize the timeout data structure.
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        // select returns 0 if timeout, 1 if input available, -1 if error.
        switch (select(FD_SETSIZE, &set, NULL, NULL, &timeout))
        {
        case -1:
            perror("select");
            close(client->connfd);
            return;
        case 0:
            printf("Timeout, closing connection\n");
            close(client->connfd);
            return;
        default:
            Http_request_header req_header;

            if (handle_http_request(client->connfd, &req_header) == -1)
            {
                printf("client closed connection or timeout\n");
                close(client->connfd);
                return;
            }
            printf("Request method: %d\n", req_header.method);
            printf("Request path: %s\n", req_header.path);
            printf("Request host: %s\n", req_header.host);
            parse_field(req_header.buffer, req_header.connection, "Connection");
            printf("Request connection: %s\n", req_header.connection);
            printf("Request body: %s\n", req_header.body);

            // check if the connection is keep-alive
            if (strncmp(req_header.connection, "keep-alive", 10) == 0)
            {
                keep_alive = true;
                res_header.connection = "keep-alive";
                printf("Connection is keep-alive\n");
            }
            else
            {
                keep_alive = false;
                res_header.connection = "close";
                printf("Connection is close\n");
            }

            // now it is time to serve the request (respond)
            if (req_header.method == HTTP_GET)
            {
                printf("GET request\n");

                // check if target file exists
                if (file_exists(req_header.path, server_config.root_dir))
                {
                    printf("File exists\n");
                    serve_request(client->connfd, req_header, res_header, server_config);
                }
                else
                {
                    printf("404 file not found\n");
                    // serve_request_404(client->connfd, req_header, res_header, server_config);
                }
                // serve_request(client->connfd, req_header, res_header, server_config);
            }
            else if (req_header.method == HTTP_POST)
            {
                // if it is just a /, then ignore it
                if ( strcmp(req_header.path, "/") == 0 )
                {
                    printf("POST request to /\n");
                    // serve_request(client->connfd, req_header, res_header, server_config);
                }
                else
                {

                    printf("POST request\n");
                    if (strcmp(get_mime_type(req_header.path), "application/json") == 0)
                    {
                        printf("POST JSON request\n");
                        save_json(req_header.path, req_header.body);
                        serve_request(client->connfd, req_header, res_header, server_config);
                        // serve_request_json(client->connfd, req_header, res_header, server_config);
                    }
                    else
                    {
                        printf("text POST request\n");
                        // serve_request(client->connfd, req_header, res_header, server_config);
                    }
                }
            }

            // If the connection is not keep-alive, or an error occurred, break the loop
            if (!keep_alive || client->connfd == -1)
            {
                break;
            }
        }
    } while (keep_alive);
}

ThreadInfo thread_list[MAX_THREADS];
int thread_count = 0;

void *handle_client_wrapper(void *arg)
{
    Thread_args *args = (Thread_args *)arg;
    Http_client *client = args->client;
    Server_config *server_config = args->server_config;

    handle_client(client, *server_config);
    printf("left handle_client\n");

    /* Update the thread status when finished. */
    // for (int i = 0; i < thread_count; i++)
    // {
    //     if (pthread_equal(pthread_self(), thread_list[i].thread_id))
    //     {
    //         thread_list[i].status = 1;
    //         break;
    //     }
    // }

    printf("about to free args\n");
    free(args); // Don't forget to free the memory when you're done

    return NULL;
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

double calculate_cpu_usage()
{
    struct rusage usage;
    struct timeval start, end;
    long sec, usec;
    double cpu_time, wall_time;

    // Get the number of CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    // Get the start time
    getrusage(RUSAGE_SELF, &usage);
    start = usage.ru_utime;

    // TODO: Insert the code you want to measure here

    // Get the end time
    getrusage(RUSAGE_SELF, &usage);
    end = usage.ru_utime;

    // Calculate the CPU time (in seconds)
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    cpu_time = sec + usec / 1e6;

    // Calculate the wall time (in seconds)
    // TODO: Replace this with the actual wall time
    wall_time = 1.0;

    // Calculate the CPU usage
    double cpu_usage = cpu_time / num_cores / wall_time * 100;

    return cpu_usage;
}

// https://stackoverflow.com/questions/8501706/how-to-get-the-cpu-usage-in-c - this helped with figuring out how to get the CPU usage

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
    printf("root dir: %s\n", server.config.root_dir);

    create_mime_db();
    ThreadPool *pool = thread_pool_create(8);

    while (1)
    {

        Http_client *client = malloc(sizeof(Http_client));
        socklen_t addr_size = sizeof(client->client_addr);
        struct rusage usage;
        struct timeval start, end, wall_start, wall_end;
        long sec, usec;
        double cpu_time, wall_time;

        // Get the number of CPU cores
        int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

        // Get the start time
        getrusage(RUSAGE_SELF, &usage);
        start = usage.ru_utime;
        gettimeofday(&wall_start, NULL); // Get the wall start time

        check_err((client->connfd = accept(server.sockfd, (SA *)&client->client_addr, &addr_size)), "Accept error");

        printf("Server: got connection from %s\n", inet_ntoa(client->client_addr.sin_addr));
        connection_count++;
        printf("Server: connection count is %d\n", connection_count);

        // ThreadInfo *info = &thread_list[thread_count++];
        Thread_args *args = malloc(sizeof(Thread_args));

        args->client = client;
        args->server_config = &server.config;

        // pthread_create(&info->thread_id, NULL, handle_client_wrapper, (void *)args);
        // printf("got back to main\n");
        // info->status = 0;
        // pthread_detach(info->thread_id);
        // handle_client(client, server.config);

        thread_pool_add_task(pool, handle_client_wrapper, (void *)args);

        // printf("\nThreads:\n");
        // for (int i = 0; i < thread_count; i++)
        // {
        //     printf("Thread %d:%ld %s\n", i, thread_list[i].thread_id, thread_list[i].status ? "Finished" : "Running");
        // }
        // Get the end time
        getrusage(RUSAGE_SELF, &usage);
        end = usage.ru_utime;
        gettimeofday(&wall_end, NULL); // Get the wall end time

        // Calculate the CPU time (in seconds)
        sec = end.tv_sec - start.tv_sec;
        usec = end.tv_usec - start.tv_usec;
        cpu_time = sec + usec / 1e6;

        // Calculate the wall time (in seconds)
        wall_time = (wall_end.tv_sec - wall_start.tv_sec) + (wall_end.tv_usec - wall_start.tv_usec) / 1e6;

        // Calculate the CPU usage
        double cpu_usage = cpu_time / num_cores / wall_time * 100;
        long mem_usage = get_memory_usage();
        printf("CPU Usage: %.2lf%%\n", cpu_usage);
        printf("Memory Usage: %ld kB\n", mem_usage);
    }

    destroy_mime_db();
    thread_pool_wait(pool);
    thread_pool_destroy(pool);
    close(server.sockfd);

    return 0;
}