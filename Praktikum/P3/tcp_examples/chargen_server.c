
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "Socket.h"
#include "ChargenOutput.h"

#include "Socket.h"

#define PORT 9019
#define BUFFER_SIZE 4096

// Function executed by each client thread.
static void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE]; // zum Empfang von Daten, die verworfen werden

    int start = 0;

    if (CheckSelectFd(client_fd) == -1)
    {
        Close(client_fd);
        return NULL;
    }

    // select, damit wir bei reciving und sending nicht blockieren, wenn client nichts sendet
    for (;;)
    {
        fd_set readable, writable;

        FD_ZERO(&readable); // empfangene Daten
        FD_ZERO(&writable); // Daten zum Senden an Kernel übergeben

        FD_SET(client_fd, &readable);
        FD_SET(client_fd, &writable);

        // select, wählt die FDs aus, die bereit sind, gelesen oder geschrieben zu werden
        int ready = Select(client_fd + 1,
                           &readable, &writable,
                           NULL, NULL);

        if (ready == -1)
        {
            if (errno == EINTR) // wenn select durch ein Signal unterbrochen wurde, erneut versuchen
                continue;
            break;
        }

        // Empfangene Daten verwerfen
        if (FD_ISSET(client_fd, &readable))
        {
            ssize_t n = Recv(client_fd, buffer,
                             sizeof(buffer), 0);

            if (n <= 0)
                break;
        }

        // Neue Zeichenfolge erzeugen und senden
        if (FD_ISSET(client_fd, &writable))
        {

            // if (SendChargenLine(client_fd, &start) == -1) break;
            if (SendDNA(client_fd, &start) == -1) break;
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
