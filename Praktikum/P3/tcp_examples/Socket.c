#include "Socket.h"
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* perror() soll den Fehlercode fuer den Aufrufer nicht veraendern. */
static void ReportError(const char *operation)
{
    int error = errno;
    perror(operation);
    errno = error;
}

void ExitOnError(ssize_t result)
{
    if (result < 0)
        exit(EXIT_FAILURE);
}

int CheckSelectFd(int fd)
{
    if (fd >= 0 && fd < FD_SETSIZE)
        return 0;

    errno = fd < 0 ? EBADF : EINVAL;
    ReportError("select descriptor");
    return -1;
}

// Ignoriert SIGPIPE, damit ein Schreibfehler auf einer geschlossenen Verbindung nicht den Prozess beendet.
int IgnoreSigpipe(void)
{
    struct sigaction action = {0};
    action.sa_handler = SIG_IGN;
    if (sigemptyset(&action.sa_mask) == -1 ||
        sigaction(SIGPIPE, &action, NULL) == -1)
    {
        ReportError("SIGPIPE");
        return -1;
    }
    return 0;
}

int Socket(int family, int type, int protocol)
{
    int result = socket(family, type, protocol);
    if (result == -1)
        ReportError("socket");
    return result;
}

int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    int result = bind(fd, addr, addrlen);
    if (result == -1)
        ReportError("bind");
    return result;
}

int Connect(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    int result = connect(fd, addr, addrlen);
    if (result == -1)
        ReportError("connect");
    return result;
}

int Listen(int fd, int backlog)
{
    int result = listen(fd, backlog);
    if (result == -1)
        ReportError("listen");
    return result;
}

int Accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    int client = accept(fd, addr, addrlen);
    if (client == -1 && errno != EINTR && errno != ECONNABORTED)
        ReportError("accept");
    return client;
}

static int AcceptInterrupted(void)
{
    return errno == EINTR || errno == ECONNABORTED;
}

int AcceptConnection(int fd)
{
    int client;
    do
    {
        client = Accept(fd, NULL, NULL);
    } while (client == -1 && AcceptInterrupted());
    return client;
}

int AcceptSelected(int fd, int *client)
{
    *client = Accept(fd, NULL, NULL);
    if (*client == -1)
        return AcceptInterrupted() ? 0 : -1;

    if (CheckSelectFd(*client) == -1)
    {
        Close(*client);
        return 0;
    }
    return 1;
}

int Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int result = setsockopt(fd, level, optname, optval, optlen);
    if (result == -1)
        ReportError("setsockopt");
    return result;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout)
{
    // Bei EINTR muss der Aufrufer seine fd_sets erneut vorbereiten.
    int result = select(nfds, readfds, writefds, exceptfds, timeout);
    if (result == -1 && errno != EINTR)
        ReportError("select");
    return result;
}

int SelectReadable(int nfds, const fd_set *watched, fd_set *readable)
{
    int count;
    do
    {
        *readable = *watched;
        count = Select(nfds, readable, NULL, NULL, NULL);
    } while (count == -1 && errno == EINTR);
    return count;
}

int Shutdown(int fd, int how)
{
    int result = shutdown(fd, how);
    if (result == -1 && errno != ECONNRESET && errno != ENOTCONN)
        ReportError("shutdown");
    return result;
}

int Close(int fd)
{
    int result = close(fd);
    if (result == -1)
        ReportError("close");
    return result;
}

ssize_t Send(int fd, const void *buf, size_t len, int flags)
{
    ssize_t n;
    do
    {
        n = send(fd, buf, len, flags);
    } while (n == -1 && errno == EINTR);
    if (n == -1 && errno != EPIPE && errno != ECONNRESET)
        ReportError("send");
    return n;
}

ssize_t Recv(int fd, void *buf, size_t len, int flags)
{
    ssize_t n;
    do
    {
        n = recv(fd, buf, len, flags);
    } while (n == -1 && errno == EINTR);
    if (n == -1 && errno != ECONNRESET)
        ReportError("recv");
    return n;
}

ssize_t Read(int fd, void *buf, size_t len)
{
    ssize_t n;
    do
    {
        n = read(fd, buf, len);
    } while (n == -1 && errno == EINTR);
    if (n == -1)
        ReportError("read");
    return n;
}

// TCP kann bei einem send() nur einen Teil des Puffers annehmen.
int SendAll(int fd, const char *buf, size_t len)
{
    size_t sent = 0;
    while (sent < len)
    {
        ssize_t n = Send(fd, buf + sent, len - sent, 0);
        if (n == -1)
            return -1;
        if (n == 0)
        {
            errno = EIO;
            ReportError("send");
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

int WriteAll(int fd, const char *buf, size_t len)
{
    size_t written = 0;
    while (written < len)
    {
        ssize_t n = write(fd, buf + written, len - written);
        if (n == -1 && errno == EINTR)
            continue;
        if (n <= 0)
        {
            if (n == 0)
                errno = EIO;
            if (errno != EPIPE)
                ReportError("write");
            return -1;
        }
        written += (size_t)n;
    }
    return 0;
}

int ListenIPv4(int port, int backlog)
{
    struct sockaddr_in server = {0};
#if defined(__FreeBSD__) || defined(__APPLE__)
    server.sin_len = sizeof(server);
#endif
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
    server.sin_port = htons(port);

    int fd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1)
        return -1;

    int reuse = 1;
    if (Setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1 ||
        Bind(fd, (const struct sockaddr *)&server, sizeof(server)) == -1 ||
        Listen(fd, backlog) == -1)
    {
        int error = errno;
        Close(fd);
        errno = error;
        return -1;
    }
    return fd;
}

/* Bis zum erfolgreichen pthread_create() gehoeren FD und Argument uns. */
void StartClientThread(int fd, void *(*handle_client)(void *))
{
    int *argument = malloc(sizeof(*argument));
    if (argument == NULL)
    {
        ReportError("malloc");
        Close(fd);
        return;
    }
    *argument = fd;

    pthread_attr_t attr;
    int error = pthread_attr_init(&attr);
    if (error == 0)
    {
        error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (error == 0)
        {
            pthread_t thread;
            error = pthread_create(&thread, &attr, handle_client, argument);
        }
        pthread_attr_destroy(&attr);
    }

    if (error != 0)
    {
        fprintf(stderr, "client thread: %s\n", strerror(error));
        free(argument);
        Close(fd);
    }
}

int Getaddrinfo(const char *hostname,
                const char *servname,
                const struct addrinfo *hints,
                struct addrinfo **result)
{
    int error = getaddrinfo(hostname, servname,
                            hints, result);

    if (error != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n",
                gai_strerror(error)); // Eigener EAI-Fehlercode
        return -1;
    }

    return 0;
}


int Getnameinfo(const struct sockaddr *addr,
                socklen_t addrlen,
                char *host,
                socklen_t hostlen,
                char *serv,
                socklen_t servlen,
                int flags)
{
    int error = getnameinfo(addr, addrlen,
                            host, hostlen,
                            serv, servlen, flags);

    if (error != 0) {
        fprintf(stderr, "getnameinfo: %s\n",
                gai_strerror(error));
        return -1;
    }

    return 0;
}


int ConnectToHost(const char *hostname, const char *service)
{
    struct addrinfo hints = {0};
    struct addrinfo *result = NULL;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (Getaddrinfo(hostname, service, &hints, &result) == -1)
        return -1;

    int sockfd = -1;

    struct addrinfo *current = result;

    while (current != NULL) {

        // Protokoll unabhängig durch ai_family
        sockfd = Socket(current->ai_family,
                        current->ai_socktype,
                        current->ai_protocol);

        if (sockfd != -1) {

            if (Connect(sockfd, current->ai_addr,
                        current->ai_addrlen) == 0)
                break;

            Close(sockfd);
            sockfd = -1;
        }

        current = current->ai_next;
    }

    freeaddrinfo(result);

    if (sockfd == -1)
        fprintf(stderr, "Could not connect to %s:%s\n",
                hostname, service);

    return sockfd;
}



int ListenIPv6(int port, int backlog)
{
    struct sockaddr_in6 server = {0};

#if defined(__FreeBSD__) || defined(__APPLE__)
    server.sin6_len = sizeof(server);
#endif

    server.sin6_family = AF_INET6;
    server.sin6_addr = in6addr_loopback;
    server.sin6_port = htons(port);

    int fd = Socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (fd == -1)
        return -1;

    int reuse = 1;
    int only_ipv6 = 1;

    if (Setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) == -1 ||

        Setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY,
                   &only_ipv6, sizeof(only_ipv6)) == -1 ||

        Bind(fd, (const struct sockaddr *)&server,
             sizeof(server)) == -1 ||

        Listen(fd, backlog) == -1) {

        int error = errno;
        Close(fd);
        errno = error;

        return -1;
    }

    return fd;
}
