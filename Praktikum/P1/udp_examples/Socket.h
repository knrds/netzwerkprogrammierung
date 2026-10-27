#ifndef SOCKET_H    
#define SOCKET_H    

#include <sys/types.h>
#include <sys/socket.h>

int Socket(int family, int type, int protocol);

int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t Sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen);

ssize_t Recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen);

int Close(int fd);

int Connect(int fd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t Send(int fd, const void *buf, size_t len, int flags);

ssize_t Recv(int fd, void *buf, size_t len, int flags);

#endif













//gcc -Wall -Wextra echo_server.c Socket.c -o echo_server
//gcc -Wall -Wextra echo_client.c Socket.c -o echo_client
