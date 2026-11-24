
#include <stdio.h>
#include <stdlib.h>

#include "Socket.h"

#define PORT 8080
#define BUFFER_SIZE 4096

// Function executed by each client thread.
static void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    for (;;)
    {
        ssize_t n = Recv(client_fd, buffer, sizeof(buffer), 0);

        if (n <= 0 || SendAll(client_fd, buffer, (size_t)n) == -1)
            break;
    }

    Close(client_fd);
    return NULL;
}

int main(void)
{
    ExitOnError(IgnoreSigpipe());

    // Create IPv4 and IPv6 listening sockets
    int listeners[2];

    listeners[0] = ListenIPv4(PORT, 16);
    ExitOnError(listeners[0]);

    listeners[1] = ListenIPv6(PORT, 16);
    ExitOnError(listeners[1]);

    // Prepare file descriptor sets
    fd_set watched;
    fd_set readable;

    FD_ZERO(&watched);

    for (int i = 0; i < 2; i++)
    {
        ExitOnError(CheckSelectFd(listeners[i]));
        FD_SET(listeners[i], &watched);
    }

    int maxfd;

    if (listeners[0] > listeners[1])
        maxfd = listeners[0];
    else
        maxfd = listeners[1];

    printf("Echo server listening on 127.0.0.1:%d and [::1]:%d\n",
           PORT, PORT);

    fflush(stdout);

    // Wait for incoming connections
    for (;;)
    {

        if (SelectReadable(maxfd + 1, &watched, &readable) == -1)
            break;

        for (int i = 0; i < 2; i++)
        {

            if (FD_ISSET(listeners[i], &readable))
            {

                int client_fd;

                int status = AcceptSelected(listeners[i], &client_fd);

                ExitOnError(status);

                if (status == 1)
                    StartClientThread(client_fd, handle_client);
            }
        }
    }

    Close(listeners[0]);
    Close(listeners[1]);

    return EXIT_FAILURE;
}
