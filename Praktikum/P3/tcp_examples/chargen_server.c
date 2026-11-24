
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "Socket.h"

#define PORT 9019
#define BUFFER_SIZE 4096
#define LINE_LENGTH 72
#define ASCII_COUNT 95

// Function executed by each client thread.
static void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE]; //zum Empfang von Daten, die verworfen werden
    char line[LINE_LENGTH + 2]; // 72 Zeichen + CR + LF (selbst erzeugte Zeichenfolge)

    int start = 0;

    if (CheckSelectFd(client_fd) == -1) {
        Close(client_fd);
        return NULL;
    }

    //select, damit wir bei reciving und sending nicht blockieren, wenn client nichts sendet
    for (;;) {
        fd_set readable, writable;

        FD_ZERO(&readable); // empfangene Daten
        FD_ZERO(&writable); // Daten zum Senden an Kernel übergeben

        FD_SET(client_fd, &readable);
        FD_SET(client_fd, &writable);

        // select, wählt die FDs aus, die bereit sind, gelesen oder geschrieben zu werden
        int ready = Select(client_fd + 1,    
                           &readable, &writable,
                           NULL, NULL);   

        if (ready == -1) {
            if (errno == EINTR) //wenn select durch ein Signal unterbrochen wurde, erneut versuchen
                continue;
            break;
        }

        // Empfangene Daten verwerfen
        if (FD_ISSET(client_fd, &readable)) {
            ssize_t n = Recv(client_fd, buffer,
                             sizeof(buffer), 0);

            if (n <= 0)
                break;
        }

        // Neue Zeichenfolge erzeugen und senden
        if (FD_ISSET(client_fd, &writable)) {

            for (int i = 0; i < LINE_LENGTH; i++)
                line[i] = 32 + (start + i) % ASCII_COUNT;

            line[LINE_LENGTH] = '\r';
            line[LINE_LENGTH + 1] = '\n';

            if (SendAll(client_fd, line, sizeof(line)) == -1)
                break;

            start = (start + 1) % ASCII_COUNT;
        }
    }

    Close(client_fd);
    return NULL;
}

int main(void)
{
    ExitOnError(IgnoreSigpipe());

    int listen_fd = ListenIPv4(PORT, 16);
    ExitOnError(listen_fd);

    printf("Chargen server listening on 127.0.0.1:%d\n", PORT);
    fflush(stdout);

    int client_fd;

    while ((client_fd = AcceptConnection(listen_fd)) != -1)
        StartClientThread(client_fd, handle_client);

    Close(listen_fd);
    return EXIT_FAILURE;
}
