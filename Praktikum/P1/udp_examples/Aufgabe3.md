# P1 – Aufgabe 2 und 3: Connected UDP und Paketgroesse

## Connected UDP

`Connect()` legt den Kommunikationspartner (IP-Adresse und Port) fest. Der
Socket bleibt UDP: kein TCP-Handshake, keine Zuverlaessigkeitsgarantie.
Danach genuegen `Send()` und `Recv()` anstelle von `Sendto()` und `Recvfrom()`.
Der Kernel filtert eingehende Datagramme nach diesem Partner. ICMP-Fehler
(z. B. Port unreachable) koennen als Socketfehler sichtbar werden; das genaue
Verhalten ist betriebssystemabhaengig.

## Maximale IPv4-UDP-Nutzdaten

65.535 Byte maximale IPv4-Gesamtlaenge - 20 Byte IPv4-Header (ohne Optionen)
- 8 Byte UDP-Header = 65.507 Byte UDP-Nutzdaten.

Server: `./echo_server`

Client: `./echo_client_connected 127.0.0.1 65507`

Client: `./echo_client_connected 127.0.0.1 65508`

Unter Linux auf Loopback erfolgreich fuer 65.507 Byte; bei 65.508 Byte
meldet send() EMSGSIZE (Message too long). Dasselbe gilt fuer Sendto().
UDP segmentiert die Anwendungsdaten nicht wie TCP. IPv4 kann Datagramme
unter bestimmten Bedingungen fragmentieren; MTU, DF und Path-MTU-Discovery
koennen die nutzbare Paketgroesse weiter begrenzen.
