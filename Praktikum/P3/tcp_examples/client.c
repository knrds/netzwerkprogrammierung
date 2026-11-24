#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Socket.h"

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    // 1. Terminal input validation
    if (argc != 3)
    {
        fprintf(stderr,
                "Usage: %s <IP address> <Port number>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    // 2. Read port number
    char *end = NULL;
    errno = 0;

    long port = strtol(argv[2], &end, 10);

    if (errno != 0 || end == argv[2] ||
        *end != '\0' || port < 1 || port > 65535)
    {
        fprintf(stderr, "Invalid port number: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    // 3. Prepare server address
    struct sockaddr_in server = {0};

#if defined(__FreeBSD__) || defined(__APPLE__)
    server.sin_len = sizeof(server);
#endif

    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, argv[1],
                  &server.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid IP address: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    ExitOnError(IgnoreSigpipe());

    int sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ExitOnError(sockfd);
    ExitOnError(CheckSelectFd(sockfd));
    ExitOnError(Connect(sockfd, (const struct sockaddr *)&server, sizeof(server)));
    fprintf(stderr, "Connected to %s:%ld\n", argv[1], port);

    char buffer[BUFFER_SIZE];
    int stdin_open = 1;
    for (;;) {
        fd_set watched;
        fd_set readable;
        FD_ZERO(&watched);
        FD_SET(sockfd, &watched);
        if (stdin_open)
            FD_SET(STDIN_FILENO, &watched);

        ExitOnError(SelectReadable(sockfd + 1, &watched, &readable));

        if (stdin_open && FD_ISSET(STDIN_FILENO, &readable)) {
            ssize_t n = Read(STDIN_FILENO, buffer, sizeof(buffer));
            ExitOnError(n);

            if (n == 0) {
                stdin_open = 0;
                ExitOnError(Shutdown(sockfd, SHUT_WR));
                fprintf(stderr, "stdin closed. Waiting for server.\n");
            } else {
                ExitOnError(SendAll(sockfd, buffer, (size_t)n));
            }
        }

        if (FD_ISSET(sockfd, &readable)) {
            ssize_t n = Recv(sockfd, buffer, sizeof(buffer), 0);
            ExitOnError(n);
            if (n == 0) {
                fprintf(stderr, "Server closed connection.\n");
                break;
            }

            ExitOnError(WriteAll(STDOUT_FILENO, buffer, (size_t)n));
        }
    }

    ExitOnError(Close(sockfd));
    return EXIT_SUCCESS;
}
