
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define PORT 9009
#define BUFFER_SIZE 4096

int main(void)
{
    // 1. Server address
    struct sockaddr_in server = {0};

#ifdef __FreeBSD__
    server.sin_len = sizeof(server);
#endif

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server.sin_port = htons(PORT);

    // 2. Create listening socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // select() uses a limited descriptor set
    if (listen_fd >= FD_SETSIZE) {
        fprintf(stderr, "Socket descriptor too large\n");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // 3. Allow address reuse
    int reuse = 1;

    if (setsockopt(listen_fd, SOL_SOCKET,
                   SO_REUSEADDR,
                   &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    // 4. Bind and listen
    if (bind(listen_fd,
             (struct sockaddr *)&server,
             sizeof(server)) == -1) {
        perror("bind");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    if (listen(listen_fd, 16) == -1) {
        perror("listen");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    printf("Discard server listening on port %d\n", PORT);

    // 5. Prepare descriptor sets
    fd_set all_fds;
    fd_set read_fds;

    FD_ZERO(&all_fds);
    FD_SET(listen_fd, &all_fds);

    int max_fd = listen_fd;
    char buffer[BUFFER_SIZE];

    // 6. Main server loop
    while (1) {

        // select() modifies its descriptor set
        read_fds = all_fds;

        int ready = select(max_fd + 1,
                           &read_fds,
                           NULL, NULL, NULL);

        if (ready == -1) {
            if (errno == EINTR)
                continue;

            perror("select");
            break;
        }

        // 7. Check all monitored descriptors
        for (int fd = 0; fd <= max_fd; fd++) {

            if (!FD_ISSET(fd, &read_fds))
                continue;

            // A new client wants to connect
            if (fd == listen_fd) {

                int client_fd = accept(
                    listen_fd, NULL, NULL
                );

                if (client_fd == -1) {
                    if (errno != EINTR)
                        perror("accept");
                    continue;
                }

                if (client_fd >= FD_SETSIZE) {
                    fprintf(stderr,
                            "Too many descriptors\n");
                    close(client_fd);
                    continue;
                }

                FD_SET(client_fd, &all_fds);

                if (client_fd > max_fd)
                    max_fd = client_fd;

                printf("Client connected: fd=%d\n",
                       client_fd);

            } else {

                // Data from an existing client
                ssize_t n = recv(
                    fd, buffer, sizeof(buffer), 0
                );

                if (n > 0) {

                    // Discard data: no send()!
                    printf("Discarded %zd bytes "
                           "from client %d\n", n, fd);

                } else if (n == 0) {

                    // Client closed connection
                    printf("Client disconnected: fd=%d\n",
                           fd);

                    close(fd);
                    FD_CLR(fd, &all_fds);

                } else if (errno != EINTR) {

                    perror("recv");
                    close(fd);
                    FD_CLR(fd, &all_fds);
                }
            }
        }

        // Recalculate highest descriptor if needed
        while (max_fd > listen_fd &&
               !FD_ISSET(max_fd, &all_fds)) {
            max_fd--;
        }
    }

    // 8. Cleanup after fatal select() error
    for (int fd = 0; fd <= max_fd; fd++) {
        if (FD_ISSET(fd, &all_fds))
            close(fd);
    }

    return EXIT_FAILURE;
}
