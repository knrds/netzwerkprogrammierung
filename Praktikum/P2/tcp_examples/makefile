CC = cc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L
CFLAGS = -g -std=c11 -Wall -Wextra -Wpedantic

all: client daytime_server discard_server echo_server

client: client.c Socket.c Socket.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -o $@ client.c Socket.c

daytime_server: daytime_server.c Socket.c Socket.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -o $@ daytime_server.c Socket.c

discard_server: discard_server.c Socket.c Socket.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -o $@ discard_server.c Socket.c

echo_server: echo_server.c Socket.c Socket.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -o $@ echo_server.c Socket.c

clean:
	rm -f client daytime_server discard_server echo_server

.PHONY: all clean
