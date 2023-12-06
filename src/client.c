// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <time.h>

// #define SERVER "10.65.255.109"
// #define PORT 7078
// #define NUM_FILES 7

// char *files[NUM_FILES] = {"/index.html", "/404.html", "/test.json", "/apps/tinyChat/app.js", "/apps/tinyChat/interface.html", "/apps/tinyChat/style.css", "/apps/tinyChat/messages.json"};

// int main(int argc, char *argv[])
// {
//     struct sockaddr_in server;
//     char server_reply[200000];
//     int sock, read_size;
//     clock_t start_t, end_t;
//     double total_t;

//     // Create socket
//     sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock == -1)
//     {
//         printf("Could not create socket");
//     }

//     server.sin_addr.s_addr = inet_addr(SERVER);
//     server.sin_family = AF_INET;
//     server.sin_port = htons(PORT);

//     // Connect to remote server
//     if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
//     {
//         perror("connect failed. Error");
//         return 1;
//     }

//     start_t = clock();
//     for (int i = 0; i < NUM_FILES; i++)
//     {
//         char message[100];
//         sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", files[i], SERVER);

//         // Send some data
//         if (send(sock, message, strlen(message), 0) < 0)
//         {
//             puts("Send failed");
//             return 1;
//         }

//         // Receive a reply from the server
//         int total_read_size = 0;
//         char *header_end = NULL;
//         while ((read_size = recv(sock, server_reply + total_read_size, 2000 - total_read_size, 0)) > 0)
//         {
//             total_read_size += read_size;
//             server_reply[total_read_size] = '\0'; // Null-terminate the string
//             header_end = strstr(server_reply, "\r\n\r\n");
//             if (header_end)
//             {
//                 break;
//             }
//         }
//         if (read_size < 0)
//         {
//             puts("recv failed");
//         }

//         // Extract the Content-Length value
//         char *content_length_pos = strstr(server_reply, "Content-Length: ");
//         if (content_length_pos)
//         {
//             int content_length = atoi(content_length_pos + strlen("Content-Length: "));
//             printf("Content-Length: %d\n", content_length);

//             // Read the rest of the data
//             while (total_read_size < content_length)
//             {
//                 read_size = recv(sock, server_reply + total_read_size, content_length - total_read_size, 0);
//                 if (read_size <= 0)
//                 {
//                     break;
//                 }
//                 total_read_size += read_size;
//             }
//         }

//         printf("Received %d bytes\n", total_read_size);
//     }

//     end_t = clock();

//     total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
//     printf("Time taken to receive all the responses : %f seconds\n", total_t);

//     close(sock);
//     return 0;
// }
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER "10.65.255.109"
#define PORT 7079
#define NUM_FILES 7

char *files[NUM_FILES] = {"/index.html", "/404.html", "/test.json", "/apps/tinyChat/app.js", "/apps/tinyChat/interface.html", "/apps/tinyChat/style.css", "/apps/tinyChat/messages.json"};


void *request_file(void *file) {
    char *path = (char *)file;

    // Create a socket and connect to the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server = { .sin_addr.s_addr = inet_addr(SERVER), .sin_family = AF_INET, .sin_port = htons(PORT) };
    connect(sock, (struct sockaddr *)&server, sizeof(server));

    // Send the GET request
    char message[100];
    sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", path, SERVER);

    // Start the timer
    clock_t start = clock();

    send(sock, message, strlen(message), 0);

    // Receive the response
    char server_reply[200000];
    int total_read_size = 0, read_size;
    while ((read_size = recv(sock, server_reply + total_read_size, sizeof(server_reply) - total_read_size, 0)) > 0) {
        total_read_size += read_size;
    }

    // Stop the timer
    clock_t end = clock();

    // Calculate the time taken
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;

    printf("Received %d bytes for %s in %f seconds\n", total_read_size, path, time_taken);

    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t threads[NUM_FILES];

    // Start the overall timer
    clock_t overall_start = clock();

    // Start a new thread for each file
    for (int i = 0; i < NUM_FILES; i++) {
        pthread_create(&threads[i], NULL, request_file, files[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_FILES; i++) {
        pthread_join(threads[i], NULL);
    }

    // Stop the overall timer
    clock_t overall_end = clock();

    // Calculate the overall time taken
    double overall_time_taken = ((double)overall_end - overall_start) / CLOCKS_PER_SEC;

    printf("Overall time taken: %f seconds\n", overall_time_taken);

    return 0;
}
