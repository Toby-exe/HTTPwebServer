#include "stdio.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define SERVER_PORT 80
#define MAX_CONNECTIONS 10
#define MAX_REQUEST_SIZE 4096

int main(int argc, char **argv)
{

    int sockfd, n;
    int sendBytes;
    struct sockaddr_in server_addr;
    char sendline[MAX_REQUEST_SIZE];
    char recvline[MAX_REQUEST_SIZE];

    //  The  domain  argument  specifies a communication domain; this selects the protocol family
    //        which will be used for communication.
    //   AF_INET      IPv4 Internet protocols                    ip(7)
    //    SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based byte streams.  An
    //                out-of-band data transmission mechanism may be supported.
    // last argument is protocol, 0 means default protocol (TCP) - http is just a protocol on top of TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_port = htons(SERVER_PORT); // port number

    // convert IPv4 and IPv6 addresses from text to binary form
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    sendBytes = strlen(sendline);

    write(sockfd, sendline, sendBytes);

    memset(recvline, 0, MAX_REQUEST_SIZE);

    while ((n = read(sockfd, recvline, MAX_REQUEST_SIZE - 1)) > 0)
    {
        printf("%s", recvline);
        memset(recvline, 0, MAX_REQUEST_SIZE);
    }
    
    exit(0);

    return 0;
}
