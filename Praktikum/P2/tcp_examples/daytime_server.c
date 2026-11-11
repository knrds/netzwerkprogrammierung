
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1313

int main(void)
{
    struct sockaddr_in server = {0};

#ifdef __FreeBSD__
    server.sin_len = sizeof(server);
#endif

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //Auf localhost binden
    server.sin_port = htons(PORT);

    signal(SIGPIPE, SIG_IGN);

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

	// TCP-Verbindungen können nach dem Schließen noch eine gewisse Zeit im Zustand TIME_WAIT verbleiben. 
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        close(fd);
        return EXIT_FAILURE;
    }


    if (bind(fd, (struct sockaddr *)&server,
             sizeof(server)) == -1) {
        perror("bind");
        close(fd);
        return EXIT_FAILURE;
    }

	// Kann nur Max 5 Clients queuen.
    if (listen(fd, 5) == -1) {
        perror("listen");
        close(fd);
        return EXIT_FAILURE;
    }


    printf("Daytime server listening on port %d\n", PORT);

    while (1) {
        int client = accept(fd, NULL, NULL);  //liefert neuen file descriptor für die Verbindung zurück, der alte bleibt bestehen

        if (client == -1) {
            if (errno == EINTR)
                continue;
            perror("accept");
            break;
        }

        time_t now = time(NULL); //aktuelle Zeit in Sekunden 
        struct tm utc; //formatierte Zeit in UTC
        char buffer[128]; 

        if (now != (time_t)-1 &&
            gmtime_r(&now, &utc) != NULL) {

            size_t len = strftime(
                buffer, sizeof(buffer),
                "%Y-%m-%d %H:%M:%S UTC\r\n",
                &utc
            );

            size_t sent = 0;

            while (sent < len) {
                ssize_t n = send(
                    client,
                    buffer + sent,
                    len - sent,
                    0
                );

                if (n < 0 && errno == EINTR)
                    continue;

                if (n <= 0) {
                    perror("send");
                    break;
                }

                sent += (size_t)n;
            }
        }



        if (shutdown(client, SHUT_WR) == -1)
			perror("shutdown client");
			
		// Auf das FIN des Clients warten
		char ack_buffer[128];

		for (;;) {
			ssize_t n = recv(client,
                     ack_buffer,
                     sizeof(ack_buffer),
                     0);

			if (n == 0) {
				printf("Client closed its sending direction.\n");
				break;
			}

			if (n < 0) {
				if (errno == EINTR)
					continue;
				perror("recv");
				break;
		}

		printf("Received %zd unexpected bytes from client\n", n);
		}

		// Socket freigeben
		if (close(client) == -1)
		perror("close client");
    }



    close(fd);
    return EXIT_FAILURE;
}
