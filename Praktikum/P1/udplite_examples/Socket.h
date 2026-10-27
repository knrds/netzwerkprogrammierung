#ifndef SOCKET_H    
#define SOCKET_H    

#include <sys/types.h>
#include <sys/socket.h>

// UDP-Lite-Optionen unter Linux
#ifndef UDPLITE_SEND_CSCOV
#define UDPLITE_SEND_CSCOV 10
#endif

#ifndef UDPLITE_RECV_CSCOV
#define UDPLITE_RECV_CSCOV 11
#endif


int Socket(int family, int type, int protocol);

int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t Sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t dest_addrlen);

ssize_t Recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *src_addrlen);

int Close(int fd);

int Connect(int fd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t Send(int fd, const void *buf, size_t len, int flags);

ssize_t Recv(int fd, void *buf, size_t len, int flags);

int Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

int ParseCoverage(const char *text);

#endif













//gcc -Wall -Wextra echo_server.c Socket.c -o echo_server
//gcc -Wall -Wextra echo_client.c Socket.c -o echo_client
