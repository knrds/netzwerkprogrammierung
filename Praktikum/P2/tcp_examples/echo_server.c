
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9007
#define BUFFER_SIZE 4096

// Send all received bytes back to the client.
static int send_all(int fd, const char *buffer, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(fd, buffer + sent, length - sent, 0);

        if (n < 0) {
            if (errno == EINTR)
                continue;

            perror("send");
            return -1;
        }

        if (n == 0) {
            fprintf(stderr, "send returned 0\n");
            return -1;
        }

        sent += (size_t)n;
    }

    return 0;
}

// Function executed by each client thread.
static void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    for (;;) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);

        if (n > 0) {
            if (send_all(client_fd, buffer, (size_t)n) == -1)
                break;

        } else if (n == 0) {
            // Client closed its sending direction.
            break;

        } else {
            if (errno == EINTR)
                continue;

            perror("recv");
            break;
        }
    }

    if (close(client_fd) == -1)
        perror("close client");

    return NULL;
}

int main(void)
{
    // Prevent SIGPIPE from terminating the process.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    // Prepare IPv4 server address.
    struct sockaddr_in server = {0};

#ifdef __FreeBSD__
    server.sin_len = sizeof(server);
#endif

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server.sin_port = htons(PORT);

    // Create listening socket.
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    int reuse = 1;

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    if (bind(listen_fd, (struct sockaddr *)&server,
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

    printf("Echo server listening on 127.0.0.1:%d\n", PORT);
    fflush(stdout);

    // Accept clients forever.
    for (;;) {
        int client_fd = accept(listen_fd, NULL, NULL);

        if (client_fd == -1) {
            if (errno == EINTR)
                continue;

            perror("accept");
            break;
        }

        // Allocate a separate argument for each thread.
        int *client_arg = malloc(sizeof(*client_arg));

        if (client_arg == NULL) {
            perror("malloc");
            close(client_fd);
            continue;
        }

        *client_arg = client_fd;

        pthread_t thread;

        int error = pthread_create(
            &thread,
            NULL,
            handle_client,
            client_arg
        );

        if (error != 0) {
            fprintf(stderr, "pthread_create: %s\n",
                    strerror(error));

            free(client_arg);
            close(client_fd);
            continue;
        }

        // Automatically release thread resources on exit.
        error = pthread_detach(thread);

        if (error != 0) {
            fprintf(stderr, "pthread_detach: %s\n",
                    strerror(error));
        }
    }

    close(listen_fd);
    return EXIT_FAILURE;
}
