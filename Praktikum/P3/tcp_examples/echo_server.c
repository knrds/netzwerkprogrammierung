#include <stdio.h>
#include <stdlib.h>

#include "Socket.h"

#define PORT 9007
#define BUFFER_SIZE 4096

// Function executed by each client thread.
static void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    for (;;) {
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
    int listen_fd = ListenIPv4(PORT, 16); // Port und Queuegröße
    ExitOnError(listen_fd);

    printf("Echo server listening on 127.0.0.1:%d\n", PORT);
    fflush(stdout);

    int client_fd;
    while ((client_fd = AcceptConnection(listen_fd)) != -1)
        StartClientThread(client_fd, handle_client);

    Close(listen_fd);
    return EXIT_FAILURE;
}
