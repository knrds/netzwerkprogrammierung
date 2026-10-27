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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "Socket.h"

#define BUFFER_SIZE (1 << 16) // 2^16 65.536	64 KiB

// ./echo_server 20 8 Senden mit Coverage 20 & Empfangen mit Mindest-Coverage 8

int main(int argc, char **argv)
{
	int fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_len;
	ssize_t len;
	char buf[BUFFER_SIZE];

	if (argc != 3)
	{
		fprintf(stderr,
				"Usage: %s <send_coverage> <recv_coverage>\n",
				argv[0]);
		return 1;
	}

	int send_coverage = ParseCoverage(argv[1]);
	int recv_coverage = ParseCoverage(argv[2]);

	fd = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDPLITE);

	Setsockopt(fd, IPPROTO_UDPLITE, UDPLITE_SEND_CSCOV,
			   &send_coverage, sizeof(send_coverage));

	Setsockopt(fd, IPPROTO_UDPLITE, UDPLITE_RECV_CSCOV,
			   &recv_coverage, sizeof(recv_coverage));

	memset((void *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

#ifdef HAVE_SIN_LEN
	server_addr.sin_len = sizeof(struct sockaddr_in);
#endif

	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // INADDR_ANY durch INADDR_LOOPBACK ersetzt
	server_addr.sin_port = htons(4177);

	Bind(fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

	for (;;)
	{

		addr_len = (socklen_t)sizeof(client_addr);

		memset((void *)&client_addr, 0, sizeof(client_addr));

		len = Recvfrom(fd, (void *)buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);

		printf("Server: %zd Bytes empfangen.\n", len);

		Sendto(fd, (const void *)buf, (size_t)len, 0, (struct sockaddr *)&client_addr, addr_len);
	}
	Close(fd);

	return (0);
}
