#include "Socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    ssize_t sent_len = sendto(fd, buf, len, flags, dest_addr, dest_addrlen);
    if (sent_len < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    return sent_len;
}

ssize_t Recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen) {
    ssize_t recv_len = recvfrom(fd, buf, len, flags, src_addr, src_addrlen);
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
    ssize_t sent_len = send(fd, buf, len, flags);
    if (sent_len < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }
    return sent_len;
}

ssize_t Recv(int fd, void *buf, size_t len, int flags) {
    ssize_t recv_len = recv(fd, buf, len, flags);
    if (recv_len < 0) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    return recv_len;
}
