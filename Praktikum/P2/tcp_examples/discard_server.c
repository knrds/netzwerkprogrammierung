#include <stdio.h>
#include <stdlib.h>

#include "Socket.h"

#define PORT 9009
#define BUFFER_SIZE 4096

int main(void)
{
    int listen_fd = ListenIPv4(PORT, 16);
    ExitOnError(listen_fd);
    ExitOnError(CheckSelectFd(listen_fd));

    printf("Discard server listening on port %d\n", PORT);

    fflush(stdout);

    // Prepare descriptor sets
    fd_set all_fds;
    fd_set read_fds;

    FD_ZERO(&all_fds);
    FD_SET(listen_fd, &all_fds);

    int max_fd = listen_fd;
    char buffer[BUFFER_SIZE];

    while (SelectReadable(max_fd + 1, &all_fds, &read_fds) != -1) {
        // Check all monitored descriptors
        for (int fd = 0; fd <= max_fd; fd++) {

            if (!FD_ISSET(fd, &read_fds))
                continue;

            // A new client wants to connect
            if (fd == listen_fd) {

                int client_fd;
                int accepted = AcceptSelected(listen_fd, &client_fd);
                if (accepted == -1)
                    goto cleanup;
                if (accepted == 0)
                    continue;

                FD_SET(client_fd, &all_fds);

                if (client_fd > max_fd)
                    max_fd = client_fd;

                printf("Client connected: fd=%d\n",
                       client_fd);

            } else {

                // Data from an existing client
                ssize_t n = Recv(
                    fd, buffer, sizeof(buffer), 0
                );

                if (n > 0) {

                    // Discard data: no send()!
                    printf("Discarded %zd bytes "
                           "from client %d\n", n, fd);

                } else {
                    if (n == 0)
                        printf("Client disconnected: fd=%d\n", fd);
                    FD_CLR(fd, &all_fds);
                    Close(fd);
                }
            }
        }

        // Recalculate highest descriptor if needed
        while (max_fd > listen_fd &&
               !FD_ISSET(max_fd, &all_fds)) {
            max_fd--;
        }
    }

cleanup:
    // Release all descriptors after a fatal listener error.
    for (int fd = 0; fd <= max_fd; fd++) {
        if (FD_ISSET(fd, &all_fds))
            Close(fd);
    }

    return EXIT_FAILURE;
}
