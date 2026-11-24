#!/bin/sh
# Öffnet ein Projekt im grafischen Code-OSS auf FreeBSD.
set -eu
fail() { printf 'FEHLER: %s\n' "$*" >&2; exit 1; }
[ "$(uname -s)" = FreeBSD ] || fail 'Bitte auf FreeBSD ausführen.'
[ "$(id -u)" -ne 0 ] || fail 'Bitte nicht als root starten.'
[ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ] || fail 'Keine grafische Sitzung gefunden. Terminal direkt am Hochschul-PC öffnen.'
PROJECT=${1:-$(pwd)}
[ -d "$PROJECT" ] || fail "Projektordner nicht gefunden: $PROJECT"
PROJECT=$(CDPATH= cd -- "$PROJECT" && pwd)
if command -v code-oss >/dev/null 2>&1; then
    exec code-oss "$PROJECT"
fi
if command -v vscode >/dev/null 2>&1; then
    exec vscode "$PROJECT"
fi
MAJOR=$(freebsd-version -u | cut -d. -f1)
PREFIX="$HOME/.local/share/freebsd-code-oss/FreeBSD-${MAJOR}-$(uname -m)/root/usr/local"
BIN="$PREFIX/share/code-oss/code-oss"
[ -x "$BIN" ] || fail 'Code-OSS nicht gefunden. Zuerst sh setup-freebsd-vscode.sh ausführen.'
# Nur für diese Anwendung, nicht dauerhaft in der Shell-Konfiguration.
export LD_LIBRARY_PATH="$PREFIX/lib:$PREFIX/lib/compat${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export XDG_DATA_DIRS="$PREFIX/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"
# Kein --no-sandbox und keine Root-Rechte. Der native Editor öffnet den Ordner.
exec "$BIN" "$PROJECT"
