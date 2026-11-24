CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

all: echo_client echo_client_connected echo_server

echo_client: echo_client.c Socket.c Socket.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ echo_client.c Socket.c

echo_client_connected: echo_client_connected.c Socket.c Socket.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ echo_client_connected.c Socket.c

echo_server: echo_server.c Socket.c Socket.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ echo_server.c Socket.c

clean:
	rm -f echo_client echo_client_connected echo_server

.PHONY: all clean
