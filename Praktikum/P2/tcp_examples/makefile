
CC = cc
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic

all: client daytime_server discard_server echo_server

client: client.c
	$(CC) $(CFLAGS) -o client client.c

daytime_server: daytime_server.c
	$(CC) $(CFLAGS) -o daytime_server daytime_server.c

discard_server: discard_server.c
	$(CC) $(CFLAGS) -o discard_server discard_server.c

echo_server: echo_server.c
	$(CC) $(CFLAGS) -pthread -o echo_server echo_server.c

clean:
	rm -f client daytime_server discard_server echo_server

.PHONY: all clean


# Alle vier Programme kompilieren
# make

# Nur einen bestimmten Server kompilieren
# make echo_server

# Alle erzeugten Programme entfernen
# make clean

# Alles frisch kompilieren
# make clean && make