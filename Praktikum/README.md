# VS Code (Code-OSS) auf dem FreeBSD-Praktikumsrechner

## Einmalig vorbereiten

Die drei Dateien aus diesem ZIP in den Hauptordner deines Repositories kopieren und committen. Nur die Skripte und diese README gehören ins Repo, nicht die heruntergeladenen Programme.

Voraussetzungen: lokale Installationen sind an der Hochschule erlaubt, eine grafische Desktop-Sitzung läuft, der echte FreeBSD-Paketmanager `pkg` ist vorhanden, das konfigurierte Paket-Repository ist erreichbar und bietet ein passendes `editors/vscode`-Paket. Im Home-Verzeichnis muss genug Platz für Download und Installation sein (ggf. mehrere GB).

## Beim Praktikum

1. Repo wie gewohnt herunterladen/aktualisieren.
2. Terminal am grafischen FreeBSD-PC öffnen und in den Repo-Ordner wechseln.
3. Einmalig ausführen:

```sh
sh setup-freebsd-vscode.sh
```

Das Skript sucht nach einem bereits installierten Code-OSS, lädt andernfalls das passende FreeBSD-Paket samt verfügbaren Abhängigkeiten und entpackt es in dein Home-Verzeichnis. Es prüft die direkt benötigten Bibliotheken und startet den Editor mit dem aktuellen Projektordner. Es gibt keine Sudo-Abfrage.

Später einfach:

```sh
sh start-freebsd-vscode.sh
```

Alternativ einen bestimmten Ordner öffnen:

```sh
sh start-freebsd-vscode.sh P2/tcp_examples
```

Nur vorbereiten, ohne GUI zu starten:

```sh
sh setup-freebsd-vscode.sh --install-only
```

`sh` funktioniert auch dann, wenn die Dateien beim Git-Checkout nicht als ausführbar markiert sind. Die Dateien müssen Unix-Zeilenenden (LF) behalten.

## C-Projekt starten

Im integrierten Terminal von Code-OSS (oder im normalen FreeBSD-Terminal):

```sh
cd P2/tcp_examples
make
```

Die konkreten Client-/Server-Befehle stehen in der Projekt-README. Code-OSS hat bereits C-Syntaxhervorhebung. Ein C-Sprachserver ist optional; Microsoft-Erweiterungen sind auf FreeBSD nicht zwingend verfügbar. Das Setup ändert deinen C-Code, Compiler und Makefiles nicht.

## Was installiert wird

Lokaler Pfad: `~/.local/share/freebsd-code-oss/FreeBSD-<Hauptversion>-<Architektur>/root/usr/local`.

Die Paketdatenbank, der Downloadcache und die temporären Dateien sind ebenfalls privat und werden nach dem Versuch entfernt. Die Systempaketdatenbank wird nicht aktualisiert. Keine Änderungen an `/usr/local`, `/etc` oder deiner Shell-Konfiguration; kein `pkg install`, kein `sudo`, keine deaktivierte Sandbox. Bei erneutem Aufruf wird eine vorhandene Installation verwendet. Auf PCs mit gemeinsamem Home-Verzeichnis kann sie bei kompatibler Systemversion und Architektur wiederverwendet werden.

## Wenn es nicht funktioniert

Die lokale Entpackmethode ist **kein offiziell unterstützter Installationsweg**. FreeBSD-Pakete können feste Systempfade, installierte Desktop-Bibliotheken, Paket-Installationsskripte oder Systemdienste voraussetzen. Auch Electron/Code-OSS ist nicht in jeder Repository-Kombination als fertiges Paket verfügbar. Ein erfolgreicher Bibliothekstest beweist noch keinen erfolgreichen GUI-Start. Das Setup wurde syntaktisch und mit simulierten Befehlen geprüft, nicht auf einem echten FreeBSD-Desktop ausgeführt.

- `Paketkatalog nicht abrufbar` / `Kein passendes vscode-Paket`: Paketquelle oder Netzwerk nicht verfügbar; Laboradministrator fragen. Das Skript ändert keine Paketquellen und lädt keine ungeprüften Drittanbieter-Binärdateien.
- `not found` / `Shared object ... not found`: benötigte Bibliothek fehlt oder ABI passt nicht.
- `Keine grafische Sitzung`: Terminal direkt am grafischen Hochschul-PC öffnen, nicht in einer gewöhnlichen SSH-Sitzung.
- `Permission denied`, `No space left` oder Sandbox-Fehler: Laborregeln, Home-Quota bzw. Systemvoraussetzungen prüfen. Keine Sicherheitsmechanismen abschalten.

Für eine vollständige Diagnose im Terminal:

```sh
freebsd-version -ku
uname -m
sh setup-freebsd-vscode.sh > vscode-setup.log 2>&1
cat vscode-setup.log
```

Wenn das Paket fehlt oder sich nicht lokal starten lässt, ist die verlässliche Alternative eine Installation durch den Administrator (`pkg install vscode`, sofern verfügbar). Erst dann ist die native systemweite Installation garantiert korrekt eingerichtet, sofern alle Abhängigkeiten verfügbar sind.

Quellen:
- https://docs.freebsd.org/en/books/handbook/desktop/
- https://www.freshports.org/editors/vscode/
- https://freebsdfoundation.org/resource/how-to-use-vs-code-on-freebsd/
- https://man.freebsd.org/cgi/man.cgi?query=pkg-fetch&sektion=8
