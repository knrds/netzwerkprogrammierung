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
#include <stdio.h>
#include <stdlib.h>
#include "Socket.h"

#define BUFFER_SIZE (1 << 16)
#define PORT 4177

int main(void)
{
    int fd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
#if defined(HAVE_SIN_LEN) || defined(__FreeBSD__) || defined(__APPLE__)
    server_addr.sin_len = sizeof(server_addr);
#endif
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(PORT);

    Bind(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Echo server listening on 127.0.0.1:%d\n", PORT);
    fflush(stdout);

    char buf[BUFFER_SIZE];
    for (;;) {
        struct sockaddr_in client_addr = {0};
        socklen_t addr_len = sizeof(client_addr);
        ssize_t len = Recvfrom(fd, buf, sizeof(buf), 0,
                               (struct sockaddr *)&client_addr, &addr_len);

        Sendto(fd, buf, (size_t)len, 0,
               (const struct sockaddr *)&client_addr, addr_len);
    }
}
