# P2 starten

FreeBSD und Windows (im WSL-Terminal) verwenden dieselben Befehle. In beiden Terminals `cd P2/tcp_examples` ausführen. In Terminal 1 zuerst `make` starten.

| Terminal 1 (Server) | Terminal 2 (Client) | Verhalten |
| --- | --- | --- |
| `./daytime_server` | `./client 127.0.0.1 1313 < /dev/null` | Gibt eine UTC-Zeile aus und endet. |
| `./discard_server` | `printf 'Hallo\n' \| ./client 127.0.0.1 9009` | Server verwirft die Bytes. |
| `./echo_server` | `printf 'Hallo\n' \| ./client 127.0.0.1 9007` | Client erhält `Hallo` zurück. |

Interaktiv geht auch `./client 127.0.0.1 9007`. **Strg+D** beendet die Eingabe. **Strg+C** stoppt den Server.
