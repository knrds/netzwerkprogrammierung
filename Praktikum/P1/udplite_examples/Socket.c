#include "Socket.h"

#if defined(__FreeBSD__)
#include <netinet/udplite.h>
#elif defined(__linux__)
/* Linux-Optionen, falls die libc-Header sie nicht bereitstellen. */
#ifndef UDPLITE_SEND_CSCOV
#define UDPLITE_SEND_CSCOV 10
#endif
#ifndef UDPLITE_RECV_CSCOV
#define UDPLITE_RECV_CSCOV 11
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int Socket(int family, int type, int protocol) {
    int fd = socket(family, type, protocol);
    if (fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(fd, addr, addrlen) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    return 0;
}

ssize_t Sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen) {
    ssize_t sent_len;
    do {
        sent_len = sendto(fd, buf, len, flags, dest_addr, dest_addrlen);
    } while (sent_len == -1 && errno == EINTR);
    if (sent_len < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    return sent_len;
}

ssize_t Recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen) {
    ssize_t recv_len;
    do {
        recv_len = recvfrom(fd, buf, len, flags, src_addr, src_addrlen);
    } while (recv_len == -1 && errno == EINTR);
    if (recv_len < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    return recv_len;
}

int Close(int fd) {
    if (close(fd) < 0) {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int Connect(int fd, const struct sockaddr *addr, socklen_t addrlen) {
    if (connect(fd, addr, addrlen) != 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    return 0;
}


ssize_t Send(int fd, const void *buf, size_t len, int flags) {
    ssize_t sent_len;
    do {
        sent_len = send(fd, buf, len, flags);
    } while (sent_len == -1 && errno == EINTR);
    if (sent_len < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }
    return sent_len;
}

ssize_t Recv(int fd, void *buf, size_t len, int flags) {
    ssize_t recv_len;
    do {
        recv_len = recv(fd, buf, len, flags);
    } while (recv_len == -1 && errno == EINTR);
    if (recv_len < 0) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    return recv_len;
}

int Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen) {
    if (setsockopt(fd, level, optname, optval, optlen) != 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    return 0;
}

size_t ParseMessageSize(const char *text) {
    char *end;
    errno = 0;
    long size = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || size < 0 || size > 65536) {
        fprintf(stderr, "Invalid message size: %s\n", text);
        exit(EXIT_FAILURE);
    }
    return (size_t)size;
}

int ParseCoverage(const char *text) {
    char *end;
    errno = 0;
    long coverage = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' ||
        coverage < 0 || coverage > 65535 || (coverage > 0 && coverage < 8)) {
        fprintf(stderr, "Invalid coverage (0 or 8..65535): %s\n", text);
        exit(EXIT_FAILURE);
    }
    return (int)coverage;
}

void SetCoverage(int fd, int send_coverage, int recv_coverage) {
    Setsockopt(fd, IPPROTO_UDPLITE, UDPLITE_SEND_CSCOV,
               &send_coverage, sizeof(send_coverage));
    Setsockopt(fd, IPPROTO_UDPLITE, UDPLITE_RECV_CSCOV,
               &recv_coverage, sizeof(recv_coverage));
}
