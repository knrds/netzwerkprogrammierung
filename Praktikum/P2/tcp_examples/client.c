
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

// Sendet alle angeforderten Bytes.
// send() darf weniger Bytes senden als verlangt.
static int send_all(int fd, const char *buf, size_t len)
{
    size_t total = 0;

    while (total < len)
    {
        ssize_t n = send(fd, buf + total, len - total, 0);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
        {
            if (n < 0)
                perror("send");
            else
                fprintf(stderr, "send returned 0\n");

            return -1;
        }

        total += (size_t)n;
    }

    return 0;
}

// Gibt alle empfangenen Bytes auf stdout aus.
static int write_all(int fd, const char *buf, size_t len)
{
    size_t total = 0;

    while (total < len)
    {
        ssize_t n = write(fd, buf + total, len - total);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0)
        {
            if (n < 0)
                perror("write");
            else
                fprintf(stderr, "write returned 0\n");

            return -1;
        }

        total += (size_t)n;
    }

    return 0;
}

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

#ifdef __FreeBSD__
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

    // 4. Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Prevent process termination from SIGPIPE
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        perror("signal");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // 5. Connect to server
    if (connect(sockfd,
                (struct sockaddr *)&server,
                sizeof(server)) == -1)
    {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Connected to %s:%ld\n", argv[1], port);

    // 6. Main communication loop
    char buffer[BUFFER_SIZE];
    int stdin_open = 1;
    int exit_status = EXIT_SUCCESS;

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);

        // Always monitor server socket
        FD_SET(sockfd, &readfds);

        // Monitor stdin until EOF
        if (stdin_open)
        {
            FD_SET(STDIN_FILENO, &readfds);
        }

        int maxfd = sockfd > STDIN_FILENO
                        ? sockfd
                        : STDIN_FILENO;

        // Wait until stdin or socket is readable
        int ready = select(maxfd + 1,           //prüft ob fd 0 bis maxfd + 1 (inklusive) bereit ist 
                           &readfds,
                           NULL,
                           NULL,
                           NULL);

        if (ready < 0)
        {
            if (errno == EINTR)
                continue;

            perror("select");
            exit_status = EXIT_FAILURE;
            break;
        }

        // 7. Handle keyboard input
        if (stdin_open &&
            FD_ISSET(STDIN_FILENO, &readfds))
        {
            ssize_t n = read(STDIN_FILENO,
                             buffer,
                             sizeof(buffer));

            if (n < 0)
            {
                if (errno == EINTR)
                    continue;

                perror("read");
                exit_status = EXIT_FAILURE;
                break;
            }

            if (n == 0)
            {
                // stdin closed: stop sending
                stdin_open = 0;

                if (shutdown(sockfd, SHUT_WR) == -1)
                {
                    perror("shutdown");
                    exit_status = EXIT_FAILURE;
                    break;
                }

                fprintf(stderr,
                        "stdin closed. Waiting for server.\n");
            }
            else
            {
                if (send_all(sockfd, buffer, (size_t)n) == -1)
                {
                    exit_status = EXIT_FAILURE;
                    break;
                }
            }
        }

        // 8. Handle server messages
        if (FD_ISSET(sockfd, &readfds))
        {
            ssize_t n = recv(sockfd,
                             buffer,
                             sizeof(buffer),
                             0);

            if (n < 0)
            {
                if (errno == EINTR)
                    continue;

                perror("recv");
                exit_status = EXIT_FAILURE;
                break;
            }

            if (n == 0)
            {
                // Server closed its sending direction
                fprintf(stderr, "Server closed connection.\n");
                break;
            }

            if (write_all(STDOUT_FILENO,
                          buffer, (size_t)n) == -1)
            {
                exit_status = EXIT_FAILURE;
                break;
            }
        }
    }

    // 9. Release socket
    if (close(sockfd) == -1)
    {
        perror("close");
        return EXIT_FAILURE;
    }

    return exit_status;
}
