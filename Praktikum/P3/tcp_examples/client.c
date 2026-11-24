
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Socket.h"

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    // 1. Terminal input validation
    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s <hostname> <service>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    // 2. Connect to server
    ExitOnError(IgnoreSigpipe());

    int sockfd = ConnectToHost(argv[1], argv[2]);

    ExitOnError(sockfd);
    ExitOnError(CheckSelectFd(sockfd));

    fprintf(stderr, "Connected to %s:%s\n",
            argv[1], argv[2]);

    // 3. Communication
    char buffer[BUFFER_SIZE];
    int stdin_open = 1;

    for (;;) {
        fd_set watched;
        fd_set readable;

        FD_ZERO(&watched);
        FD_SET(sockfd, &watched);

        if (stdin_open)
            FD_SET(STDIN_FILENO, &watched);

        ExitOnError(SelectReadable(sockfd + 1,
                                   &watched, &readable));

        if (stdin_open && FD_ISSET(STDIN_FILENO, &readable)) {

            ssize_t n = Read(STDIN_FILENO, buffer,
                             sizeof(buffer));

            ExitOnError(n);

            if (n == 0) {
                stdin_open = 0;

                ExitOnError(Shutdown(sockfd, SHUT_WR));

                fprintf(stderr,
                        "stdin closed. Waiting for server.\n");

            } else {
                ExitOnError(SendAll(sockfd, buffer, (size_t)n));
            }
        }

        if (FD_ISSET(sockfd, &readable)) {

            ssize_t n = Recv(sockfd, buffer,
                             sizeof(buffer), 0);

            ExitOnError(n);

            if (n == 0) {
                fprintf(stderr, "Server closed connection.\n");
                break;
            }

            ExitOnError(WriteAll(STDOUT_FILENO,
                                 buffer, (size_t)n));
        }
    }

    ExitOnError(Close(sockfd));

    return EXIT_SUCCESS;
}
