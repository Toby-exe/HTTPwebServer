/**
 * @file server.c
 * @authors Tobias Wondwossen, Jayden Mingle
 *
 * @date 2023-12-06
 */

#include "server.h"

const char html_start[] = "<html><head><style>"
                          "body {font-family: Arial, sans-serif; margin:0; padding:0; background-color: #f0f0f0;}"
                          "ul {list-style-type: none; margin: 0; padding: 0;}"
                          "li {padding: 10px 0; border-bottom: 1px solid #ddd;}"
                          "li:last-child {border-bottom: none;}"
                          "li a {text-decoration: none; color: #333; display: block; padding: 10px;}"
                          "li a:hover {background-color: #ddd;}"
                          "</style></head><body><ul>";

const char html_end[] = "</ul></body></html>";

const char page_404[] = "<!DOCTYPE html>\r\n"
                        "<html>\r\n"
                        "<head>\r\n"
                        "    <title>404 Not Found</title>\r\n"
                        "    <style>\r\n"
                        "        body { background-color: #fafafa; font-family: Arial, sans-serif; color: #333; margin: 0; padding: 0; }\r\n"
                        "        .container { max-width: 600px; margin: 0 auto; padding: 20px; text-align: center; }\r\n"
                        "        h1 { font-size: 48px; color: #888; }\r\n"
                        "        p { font-size: 24px; }\r\n"
                        "        a { color: #0645ad; text-decoration: none; }\r\n"
                        "    </style>\r\n"
                        "</head>\r\n"
                        "<body>\r\n"
                        "    <div class=\"container\">\r\n"
                        "        <h1>404 Not Found</h1>\r\n"
                        "        <p>The requested URL was not found on this server or you do not have access</p>\r\n"
                        "        <p><a href=\"/\">Go to Home</a></p>\r\n"
                        "    </div>\r\n"
                        "</body>\r\n"
                        "</html>\r\n";
/**
 * @brief Checks if a value is -1 and prints a message if it is
 *
 * This function takes an integer and a message as input. If the integer is -1, it prints
 * the provided message.
 *
 * @param[in] val The integer to be checked
 * @param[in] msg The message to be printed if the integer is -1
 * @return This function does not return a value
 */
void check_err(int val, char *msg)
{
    if (val == -1)
    {
        printf("%s\n", msg);
    }
}

/**
 * @brief Parses a specific field from a source string and copies it to a destination string
 *
 * This function takes a source string, a destination string, and a field name as input. It finds
 * the field in the source string, and copies its value to the destination string. If the field is
 * not found, it prints a message and returns.
 *
 * @param[in] src The source string to parse
 * @param[out] des The destination string where the field value will be copied
 * @param[in] field The field to be parsed from the source string
 * @return This function does not return a value
 */
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

/**
 * @brief Handles an HTTP request
 *
 * This function takes a connection file descriptor and an HTTP request header as input. It
 * reads the request from the connection, parses the request method and path, and fills the
 * request header structure accordingly. If there is an error during the process, it prints
 * an error message and returns -1.
 *
 * @param[in] connfd The connection file descriptor
 * @param[out] req_header The HTTP request header structure to be filled
 * @return 0 if the request was handled successfully, -1 otherwise
 */
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

/**
 * @brief Sends an HTTP response
 *
 * This function takes a connection file descriptor, an HTTP response header,
 * and a file size as input. It constructs the response header and sends it
 * through the connection. If there is an error during the process, it prints
 * an error message.
 *
 * @param[in] connfd The connection file descriptor
 * @param[in] res_header The HTTP response header structure
 * @param[in] file_size The size of the file to be sent in the response
 * @return This function does not return a value
 */
void send_response(const int connfd, Http_response_header res_header, long file_size)
{
    char response[MAX_HEADER_SIZE];

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
    }
}

/**
 * @brief Serves a file over HTTP
 *
 * This function takes a connection file descriptor, an HTTP request header, an HTTP
 * response header, and a server configuration as input. It constructs the path to
 * the requested file, opens the file, gets its size, sends the response header, sends
 * the file, and finally closes the file. If there is an error during the process, it
 * prints an error message and returns.
 *
 * @param[in] connfd The connection file descriptor
 * @param[in] req_header The HTTP request header structure
 * @param[out] res_header The HTTP response header structure to be filled
 * @param[in] server_config The server configuration
 * @return This function does not return a value
 */
void serve_file(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config)
{
    char response[MAX_HEADER_SIZE];
    char file_path[MAX_PATH_SIZE * 2];
    struct stat file_stat;
    int fd;
    long file_size = 0;

    memset(&res_header.content_type, 0, sizeof(res_header.content_type));
    memset(response, 0, sizeof(response));
    memset(file_path, 0, sizeof(file_path));

    strcpy(res_header.content_type, get_mime_type(req_header.path));

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

    send_response(connfd, res_header, file_size);

    // Send the file
    if (sendfile(connfd, fd, NULL, file_size) == -1)
    {
        perror("sendfile");
        return;
    }

    // Close the file
    close(fd);
}

/**
 * @brief Serves a directory listing over HTTP
 *
 * This function takes a connection file descriptor, an HTTP request header, an HTTP response header,
 * and a server configuration as input. It constructs the path to the requested directory, opens the
 * directory, reads its entries, and sends a response with an HTML page that lists the directory entries.
 * If there is an error during the process, it prints an error message and returns.
 *
 * @param[in] connfd The connection file descriptor
 * @param[in] req_header The HTTP request header structure
 * @param[out] res_header The HTTP response header structure to be filled
 * @param[in] server_config The server configuration
 * @return This function does not return a value
 */
void serve_dir(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config)
{
    char response[MAX_HEADER_SIZE];
    char file_path[256];
    long file_size = 0;
    char *page_buffer;

    memset(&res_header.content_type, 0, sizeof(res_header.content_type));
    memset(response, 0, sizeof(response));
    memset(file_path, 0, sizeof(file_path));

    // Construct the full path
    char full_path[4096]; // Adjust size as needed
    snprintf(full_path, sizeof(full_path), "%s%s", server_config.root_dir, req_header.path);

    // Open the directory
    DIR *dir = opendir(full_path);
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    char html_body[1024 * 1024] = {0}; // 1mb

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
    char html_response[DIR_LISTING_PAGE_SZ];
    snprintf(html_response, sizeof(html_response), "%s%s%s", html_start, html_body, html_end);

    // Set the response fields
    strcpy(res_header.content_type, "text/html");
    file_size = strlen(html_response);

    size_t buffer_size = strlen(html_start) + strlen(html_body) + strlen(html_end) + 1;
    page_buffer = malloc(buffer_size);
    if (page_buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for page_buffer.\n");
        return;
    }
    snprintf(page_buffer, buffer_size, "%s%s%s", html_start, html_body, html_end);

    send_response(connfd, res_header, file_size);

    printf("sending page: %s\n", page_buffer);
    // Send the file
    if (send(connfd, page_buffer, strlen(page_buffer), 0) == -1)
    {
        perror("send");
        return;
    }
}

/**
 * @brief Serves an HTTP request
 *
 * This function takes a connection file descriptor, an HTTP request header, an HTTP response header, and
 * a server configuration as input. It constructs the full path to the requested resource, checks if it's a
 * file or a directory, and calls the appropriate function to serve the resource. If there is an error during
 * the process, it prints an error message and returns.
 *
 * @param[in] connfd The connection file descriptor
 * @param[in] req_header The HTTP request header structure
 * @param[out] res_header The HTTP response header structure to be filled
 * @param[in] server_config The server configuration
 * @return This function does not return a value
 */
void serve_request(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config)
{
    // create state machine either in serve file or server dir
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
        // current_state = SERVE_DIR;
        serve_dir(connfd, req_header, res_header, server_config);
    }
    else if (S_ISREG(path_stat.st_mode))
    {
        // current_state = SERVE_FILE;
        serve_file(connfd, req_header, res_header, server_config);
    }
}

/**
 * @brief Serves a 404 Not Found HTTP response
 *
 * This function takes a connection file descriptor, an HTTP request header, an HTTP response header, and a
 * server configuration as input. It sets the response status to 404 Not Found, sets the requested path to
 * the 404 error page, and calls the serve_request function to serve the error page.
 *
 * @param[in] connfd The connection file descriptor
 * @param[in] req_header The HTTP request header structure
 * @param[out] res_header The HTTP response header structure to be filled
 * @param[in] server_config The server configuration
 * @return This function does not return a value
 */
void serve_request_404(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config)
{
    memset(&res_header.content_type, 0, sizeof(res_header.content_type));
    strcpy(res_header.status_code, "404");
    strcpy(res_header.status_message, "Not Found");
    strcpy(res_header.additional_headers, "Server: tinyserver\r\n");
    strcpy(res_header.connection, "close");
    strcpy(req_header.path, "/404.html");

    // Create the 404 page
    // char *page_404 = "<!DOCTYPE html>\r\n"
    //                  "<html><head><title>404 Not Found</title></head>\r\n"
    //                  "<body>\r\n"
    //                  "<h1>404 Not Found</h1>\r\n"
    //                  "<p>The requested URL was not found on this server. Please check the URL or contact support if you need assistance.</p>\r\n"
    //                  "</body></html>\r\n";

    // Calculate the size of the 404 page
    long file_size = strlen(page_404);

    // Call send_response
    send_response(connfd, res_header, file_size);

    // Send the 404 page
    ssize_t len = strlen(page_404);
    ssize_t bytes_sent = send(connfd, page_404, len, 0);
    if (bytes_sent < 0)
    {
        perror("sending 404 page failed");
    }
}
/**
 * @brief Handles an HTTP POST request
 *
 * This function takes a connection file descriptor, an HTTP request header, an HTTP response header, and
 * a server configuration as input. It checks the path and the MIME type of the request. If the path is "/",
 * it ignores the request. If the MIME type is "application/json", it saves the JSON body and serves the request.
 * Otherwise, it just prints a message.
 *
 * @param[in] connfd The connection file descriptor
 * @param[in] req_header The HTTP request header structure
 * @param[out] res_header The HTTP response header structure to be filled
 * @param[in] server_config The server configuration
 * @return This function does not return a value
 */
void http_post_handler(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config)
{
    printf("POST request\n");

    // if it is just a /, then ignore it
    if (strcmp(req_header.path, "/") == 0)
    {
        printf("POST request to /\n");
        serve_request_404(connfd, req_header, res_header, server_config);
        // serve_request(client->connfd, req_header, res_header, server_config);
    }
    else
    {

        if (strcmp(get_mime_type(req_header.path), "application/json") == 0)
        {
            printf("POST JSON request\n");
            save_json(req_header.path, req_header.body);
            serve_request(connfd, req_header, res_header, server_config);
        }
        else
        {
            printf("text POST request\n");
            save_file(req_header.path, req_header.body);
            serve_request(connfd, req_header, res_header, server_config);
        }
    }
}

/**
 * @brief Handles an HTTP GET request
 *
 * This function takes a connection file descriptor, an HTTP request header, an HTTP response header, and a server configuration as input. It checks if the requested file exists. If it does, it serves the file. If it doesn't, it serves a 404 Not Found response.
 *
 * @param[in] connfd The connection file descriptor
 * @param[in] req_header The HTTP request header structure
 * @param[out] res_header The HTTP response header structure to be filled
 * @param[in] server_config The server configuration
 * @return This function does not return a value
 */
void http_get_handler(const int connfd, Http_request_header req_header, Http_response_header res_header, Server_config server_config)
{
    printf("GET request\n");
    // check if target file exists
    if (file_exists(req_header.path, server_config.root_dir))
    {
        printf("File exists\n");
        serve_request(connfd, req_header, res_header, server_config);
    }
    else
    {

        printf("404 file not found\n");
        serve_request_404(connfd, req_header, res_header, server_config);
    }
}

/**
 * @brief Handles an HTTP client
 *
 * This function takes an HTTP client and a server configuration as input. It sets up a response
 * header, and then enters a loop where it waits for a request from the client. If a request is received,
 * it handles the request based on its method (GET or POST). If the connection is set to keep-alive, it
 * continues to wait for more requests; otherwise, it closes the connection.
 *
 * @param[in] client The HTTP client to be handled
 * @param[in] server_config The server configuration
 * @return This function does not return a value
 */
void handle_client_persistent(Http_client *client, Server_config server_config)
{
    printf("\033[33mThread %ld\033[0m\n", pthread_self());

    Http_response_header res_header;
    bool keep_alive = false;
    fd_set set;
    struct timeval timeout;

    memset(&res_header, 0, sizeof(res_header));

    strcpy(res_header.status_code, "200");
    strcpy(res_header.status_message, "OK");
    strcpy(res_header.additional_headers, "Keep-Alive: timeout=10\r\nServer: tinyserver\r\n");
    strcpy(res_header.connection, "close");

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
            // check if the connection is keep-alive
            if (strncmp(req_header.connection, "keep-alive", 10) == 0)
            {
                keep_alive = true;
                // res_header.connection = "keep-alive";
                strcpy(res_header.connection, "keep-alive");
                printf("Connection is keep-alive\n");
            }
            else
            {
                keep_alive = false;
                // res_header.connection = "close";
                strcpy(res_header.connection, "close");
                memset(&res_header.additional_headers, 0, sizeof(res_header.additional_headers));
                strcpy(res_header.additional_headers, "Server: tinyserver\r\n");
                printf("Connection is close\n");
            }

            // now it is time to serve the request (respond)
            if (req_header.method == HTTP_GET)
            {
                http_get_handler(client->connfd, req_header, res_header, server_config);
            }
            else if (req_header.method == HTTP_POST)
            {
                http_post_handler(client->connfd, req_header, res_header, server_config);
            }
        }
    } while (keep_alive);
}

void handle_client(Http_client *client, Server_config server_config)
{
    printf("\033[33mThread %ld\033[0m\n", pthread_self());

    Http_response_header res_header;
    memset(&res_header, 0, sizeof(res_header));

    strcpy(res_header.status_code, "200");
    strcpy(res_header.status_message, "OK");
    strcpy(res_header.additional_headers, "Server: tinyserver\r\n");
    strcpy(res_header.connection, "close");

    Http_request_header req_header;

    if (handle_http_request(client->connfd, &req_header) == -1)
    {
        printf("client closed connection or timeout\n");
        close(client->connfd);
        return;
    }

    // now it is time to serve the request (respond)
    if (req_header.method == HTTP_GET)
    {
        http_get_handler(client->connfd, req_header, res_header, server_config);
    }
    else if (req_header.method == HTTP_POST)
    {
        http_post_handler(client->connfd, req_header, res_header, server_config);
    }

    close(client->connfd);
}

/**
 * @brief A wrapper function for handling an HTTP client in a new thread
 *
 * This function takes a pointer to a Thread_args structure as input, which contains an HTTP client and
 * a server configuration. It calls the handle_client function to handle the client, and then frees the
 * memory allocated for the Thread_args structure.
 *
 * @param[in] arg A pointer to a Thread_args structure containing the HTTP client to be handled and the server configuration
 * @return This function does not return a value
 */
void *handle_client_wrapper(void *arg)
{
    Thread_args *args = (Thread_args *)arg;
    Http_client *client = args->client;
    Server_config *server_config = args->server_config;

    if (server_config->enable_keep_alive == ON)
        handle_client_persistent(client, *server_config);
    else
        handle_client(client, *server_config);

    free(args); // Don't forget to free the memory when you're done

    return NULL;
}

/**
 * @brief Starts the HTTP server
 *
 * This function takes an HTTP server and command line arguments as input. It sets the server configuration with
 * default or provided values, creates a socket, binds it to the specified port, and starts listening for connections.
 * If there is an error during the process, it prints an error message and returns.
 *
 * @param[in] server The HTTP server to be started
 * @param[in] argc The number of command line arguments
 * @param[in] argv The command line arguments
 * @return This function does not return a value
 */
void start_server(Http_server *server, int argc, char *argv[])
{
    // Set default values
    // Set default values
    strcpy(server->config.port, DEFAULT_PORT);
    strcpy(server->config.root_dir, DEFAULT_ROOT_DIR);
    server->config.enable_mt = ON;
    server->config.enable_keep_alive = OFF;
    server->config.num_threads = DEFAULT_NUM_THREADS;
    server->config.enable_stats = OFF;

    // Override with command line arguments if provided
    int opt;
    while ((opt = getopt(argc, argv, "p:r:m:k:t:s:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            strcpy(server->config.port, optarg);
            break;
        case 'r':
            strcpy(server->config.root_dir, optarg);
            break;
        case 'm':
            server->config.enable_mt = (strcmp(optarg, "on") == 0) ? ON : OFF;
            break;
        case 'k':
            server->config.enable_keep_alive = (strcmp(optarg, "on") == 0) ? ON : OFF;
            break;
        case 't':
            server->config.num_threads = atoi(optarg);
            break;
        case 's':
            server->config.enable_stats = (strcmp(optarg, "on") == 0) ? ON : OFF;
            break;
        default:
            fprintf(stderr, "Usage: %s [-p port] [-r root_dir] [-m enable_mt] [-k enable_keep_alive] [-t num_threads] [-s enable_stats]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    check_err((server->sockfd = socket(AF_INET, SOCK_STREAM, 0)), "Socket error");

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(atoi(server->config.port));

    int optval = 1;
    check_err(setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)), "Setsockopt error");

    check_err(bind(server->sockfd, (SA *)&server->server_addr, sizeof(server->server_addr)), "Bind error");
    check_err(listen(server->sockfd, BACKLOG), "Listen error");

    printf("Server: waiting for connection on port %s...\n", server->config.port);
    printf("You can access it at: \033[32m\033[4mhttp://10.65.255.109:%s/\033[0m\n", server->config.port);
    printf("root dir: %s\n", server->config.root_dir);
}

/**
 * @brief Accepts a client connection and adds it to the thread pool
 *
 * This function takes an HTTP server, a thread pool, and a pointer to the connection count as input.
 * It accepts a client connection, increments the connection count, and adds a task to the thread pool to
 * handle the client. The task is a call to the handle_client_wrapper function with a Thread_args structure
 * containing the client and the server configuration.
 *
 * @param[in] server The HTTP server
 * @param[in] pool The thread pool
 * @param[out] connection_count A pointer to the connection count
 * @return This function does not return a value
 */
void accept_client(Http_server *server, ThreadPool *pool, int *connection_count)
{
    Http_client *client = malloc(sizeof(Http_client));
    socklen_t addr_size = sizeof(client->client_addr);

    check_err((client->connfd = accept(server->sockfd, (SA *)&client->client_addr, &addr_size)), "Accept error");

    printf("Server: got connection from %s\n", inet_ntoa(client->client_addr.sin_addr));
    (*connection_count)++;
    printf("Server: connection count is %d\n", *connection_count);

    Thread_args *args = malloc(sizeof(Thread_args));
    args->client = client;
    args->server_config = &server->config;

    if (server->config.enable_mt == OFF)
        if (server->config.enable_keep_alive == ON)
            handle_client_persistent(client, server->config);
        else
            handle_client(client, server->config);
    else
        thread_pool_add_task(pool, handle_client_wrapper, (void *)args);
}

void calculate_usage(struct timeval start, struct timeval wall_start)
{
    struct rusage usage;
    struct timeval end, wall_end;
    long sec, usec;
    double cpu_time, wall_time;

    // Get the number of CPU cores
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

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

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return;
    }

    // Initialize the destination address and port
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(18000);                      // The port number to send data to
    dest_addr.sin_addr.s_addr = inet_addr("10.65.255.109"); // The destination IP address

    // Format the usage statistics as a string
    char buffer[256];
    snprintf(buffer, 256, "CPU Usage: %.2lf%%\nMemory Usage: %ld kB\n", cpu_usage, mem_usage);

    // Send the usage statistics to the destination socket
    int bytes = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (bytes < 0)
    {
        perror("sendto");
        close(sockfd);
        return;
    }
    printf("Sent %d bytes to port %d\n", bytes, 8080);

    // Close the socket
    close(sockfd);
}
