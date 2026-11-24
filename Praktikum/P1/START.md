# P1 starten

FreeBSD und Windows (im WSL-Terminal) verwenden dieselben Befehle. Beide Terminals starten im Ordner `Praktikum`.

| Variante | Terminal 1 (Server) | Terminal 2 (Client) |
| --- | --- | --- |
| UDP-Echo | `cd P1/udp_examples && make && ./echo_server` | `cd P1/udp_examples && ./echo_client 127.0.0.1 64` |
| Connected UDP | derselbe UDP-Server | `./echo_client_connected 127.0.0.1 64` im Ordner `P1/udp_examples` |
| UDP-Lite-Echo | `cd P1/udplite_examples && make && ./echo_server 8 8` | `cd P1/udplite_examples && ./echo_client 127.0.0.1 8 8 64` |
| Connected UDP-Lite | derselbe UDP-Lite-Server | `./echo_client_connected 127.0.0.1 8 8 64` im Ordner `P1/udplite_examples` |

Alle Clients erwarten ein Echo auf `127.0.0.1:4177`. Die letzte Zahl ist die Nutzdatengröße. Bei UDP-Lite stehen davor Sende- und Empfangs-Coverage. `0 0` bedeutet volle Prüfsummenabdeckung. Server mit **Strg+C** stoppen.
