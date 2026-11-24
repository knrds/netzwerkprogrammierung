#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int Socket(int family, int type, int protocol);
int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen);
int Connect(int fd, const struct sockaddr *addr, socklen_t addrlen);
int Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
ssize_t Sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *addr, socklen_t addrlen);
ssize_t Recvfrom(int fd, void *buf, size_t len, int flags,
                 struct sockaddr *addr, socklen_t *addrlen);
ssize_t Send(int fd, const void *buf, size_t len, int flags);
ssize_t Recv(int fd, void *buf, size_t len, int flags);
int Close(int fd);

/* Hilfsfunktionen fuer die Kommandozeilenargumente. */
size_t ParseMessageSize(const char *text);

#endif
