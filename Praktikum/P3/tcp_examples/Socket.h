#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

/* Fehler ergeben -1 mit errno. Uebliche Verbindungsabbrueche bleiben still. */
/* Beendet den Prozess bei einem negativen, bereits gemeldeten Rueckgabewert.
 * Gedacht fuer den Einzelclient und fuer die Server-Initialisierung. */
void ExitOnError(ssize_t result);
/* Prueft einen FD vor FD_SET. 0 ist gueltig, -1 meldet einen Fehler. */
int CheckSelectFd(int fd);
int IgnoreSigpipe(void); /* sigemptyset(), sigaction(): 0 oder -1. */
int Socket(int family, int type, int protocol);
int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen);
int Connect(int fd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int fd, int backlog);
/* EINTR/ECONNABORTED bleiben still; der Aufrufer entscheidet ueber Retry. */
int Accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
/* Wartet mit accept(); versucht EINTR/ECONNABORTED erneut. FD oder -1. */
int AcceptConnection(int fd);
/* Ein accept() nach select(). 1 = FD in *client, 0 = weiter beobachten
 * (Abbruch, Signal oder zu grosser FD), -1 = Listenerfehler. */
int AcceptSelected(int fd, int *client);
int Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
/* Bei EINTR vor einem neuen Aufruf Sets und Timeout neu vorbereiten. */
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout);
/* Lesendes select() ohne Timeout. Kopiert watched auch bei EINTR neu
 * nach readable. Gibt die Anzahl bereiter FDs oder -1 zurueck. */
int SelectReadable(int nfds, const fd_set *watched, fd_set *readable);
int Shutdown(int fd, int how);
int Close(int fd);
ssize_t Send(int fd, const void *buf, size_t len, int flags);
ssize_t Recv(int fd, void *buf, size_t len, int flags);
ssize_t Read(int fd, void *buf, size_t len);
/* Sendet len Bytes von buf an fd mit send(). 0 bei Erfolg, -1 bei Fehler
 * (errno bleibt erhalten). Bei einem Fehler kann ein Teil gesendet sein. */
int SendAll(int fd, const char *buf, size_t len);
/* Schreibt len Bytes von buf auf fd mit write(). 0 bei Erfolg, -1 bei
 * Fehler (errno bleibt erhalten). Bei Fehler ist Teilausgabe moeglich. */
int WriteAll(int fd, const char *buf, size_t len);

/* Startet einen TCP-Listener auf 127.0.0.1:port mit Warteschlange backlog.
 * socket(), setsockopt(), bind(), listen(); FD bei Erfolg, sonst -1.
 * Bei Fehler nach socket() wird der FD mit close() freigegeben. */
int ListenIPv4(int port, int backlog);

/* Startet mit pthread_create() einen losgeloesten Thread. Der Handler
 * uebernimmt den FD und ein damit belegtes int*. Bei einem Startfehler
 * meldet der Wrapper den Fehler und gibt FD und Speicher frei. */
void StartClientThread(int fd, void *(*handle_client)(void *));

#endif
