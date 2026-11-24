#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Socket.h"

#define PORT 1313

int main(void)
{
    ExitOnError(IgnoreSigpipe());
    int fd = ListenIPv4(PORT, 5);
    ExitOnError(fd);

    printf("Daytime server listening on port %d\n", PORT);
    fflush(stdout);

    int client;
    while ((client = AcceptConnection(fd)) != -1) {
        time_t now = time(NULL);
        struct tm utc;
        char buffer[128];

        if (now != (time_t)-1 && gmtime_r(&now, &utc) != NULL) {
            size_t len = strftime(buffer, sizeof(buffer),
                                  "%Y-%m-%d %H:%M:%S UTC\r\n", &utc);
            if (len > 0)
                SendAll(client, buffer, len);
        }
        // Daytime sendet die Zeit und beendet die Verbindung.
        Close(client);
    }

    Close(fd);
    return EXIT_FAILURE;
}
