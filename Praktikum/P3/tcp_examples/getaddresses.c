
#include <stdio.h>
#include <stdlib.h>

#include "Socket.h"

int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Configure address resolution
    struct addrinfo hints = {0};

    hints.ai_family = AF_UNSPEC; //Egal ob IPv4 oder IPv6
    hints.ai_socktype = SOCK_STREAM;  

    // Resolve hostname
    struct addrinfo *result = NULL; // Pointer auf das Result und wir übergeben später die Adresse des Pointers damit er auf das ergebnis ziegt

    ExitOnError(Getaddrinfo(argv[1], NULL,
                            &hints, &result));

    // Iterate through all addresses
    struct addrinfo *current = result;  // List an Adressen in binary
    int status = EXIT_SUCCESS;

    while (current != NULL) {

        char host[1025];

        if (Getnameinfo(current->ai_addr,
                        current->ai_addrlen,
                        host, sizeof(host),
                        NULL, 0, NI_NUMERICHOST) == 0) {

            if (current->ai_family == AF_INET)
                printf("IPv4: %s\n", host);

            else if (current->ai_family == AF_INET6)
                printf("IPv6: %s\n", host);

        } else {
            status = EXIT_FAILURE;
        }

        current = current->ai_next;
    }

    freeaddrinfo(result);

    return status;
}
