/*-
 * Copyright (c) 2013 Michael Tuexen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Socket.h"

#define BUFFER_SIZE (1 << 16)
#define MESSAGE_SIZE 65507
#define PORT 4177

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s <server-ip> [bytes]\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t message_size = argc == 3 ? ParseMessageSize(argv[2]) : MESSAGE_SIZE;

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
#if defined(HAVE_SIN_LEN) || defined(__FreeBSD__) || defined(__APPLE__)
    server_addr.sin_len = sizeof(server_addr);
#endif
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int fd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // Ein verworfenes Paket soll den Test nicht endlos blockieren.
    struct timeval timeout = {2, 0};
    Setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    char buf[BUFFER_SIZE];
    memset(buf, 'A', message_size);
    Connect(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    Send(fd, buf, message_size, 0);
    ssize_t len = Recv(fd, buf, sizeof(buf), 0);
    printf("Received %zd bytes from connected server.\n", len);

    Close(fd);
    return EXIT_SUCCESS;
}
