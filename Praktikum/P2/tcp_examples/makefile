CC = cc
CFLAGS = -Wall -Wextra -std=c11

all: client daytime_server discard_server echo_server

client:
	$(CC) $(CFLAGS) client.c -o client

daytime_server:
	$(CC) $(CFLAGS) daytime_server.c -o daytime_server

discard_server:
	$(CC) $(CFLAGS) discard_server.c -o discard_server

echo_server:
	$(CC) $(CFLAGS) -pthread echo_server.c -o echo_server

clean:
	rm -f client daytime_server discard_server echo_server


# Alle vier Programme kompilieren
# make

# Nur einen bestimmten Server kompilieren
# make echo_server

# Alle erzeugten Programme entfernen
# make clean

# Alles frisch kompilieren
# make clean && make
