#!/bin/sh
# Installiert Code-OSS versuchsweise lokal, ohne Root und ohne Systemänderungen.
set -eu
umask 077
fail() { printf '\nFEHLER: %s\n' "$*" >&2; exit 1; }
[ "$(uname -s)" = FreeBSD ] || fail 'Dieses Setup muss auf FreeBSD ausgeführt werden.'
[ "$(id -u)" -ne 0 ] || fail 'Bitte als normaler Benutzer ausführen, nicht als root.'
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
[ -f "$HERE/start-freebsd-vscode.sh" ] || fail 'Das Startskript muss neben dem Setup liegen.'
if [ "${1:-}" = --install-only ]; then
    INSTALL_ONLY=yes
    shift
else
    INSTALL_ONLY=no
fi
PROJECT=${1:-$(pwd)}
[ -d "$PROJECT" ] || fail "Projektordner nicht gefunden: $PROJECT"
PROJECT=$(CDPATH= cd -- "$PROJECT" && pwd)
MAJOR=$(freebsd-version -u | cut -d. -f1)
BASE="$HOME/.local/share/freebsd-code-oss/FreeBSD-${MAJOR}-$(uname -m)"
PREFIX="$BASE/root/usr/local"
if command -v code-oss >/dev/null 2>&1 || command -v vscode >/dev/null 2>&1 || [ -x "$PREFIX/share/code-oss/code-oss" ]; then
    printf 'Code-OSS bereits vorhanden; kein erneuter Download.\n'
else
    PKG=/usr/local/sbin/pkg
    if [ ! -x "$PKG" ]; then PKG=/usr/local/sbin/pkg-static; fi
    [ -x "$PKG" ] || fail 'Der echte pkg-Paketmanager fehlt. Bitte Laboradministrator kontaktieren; kein Bootstrap als root.'
    mkdir -p "$BASE"
    WORK=$(mktemp -d "$BASE/setup.XXXXXXXX")
    trap 'rm -rf -- "$WORK"' EXIT
    trap 'exit 130' INT
    trap 'exit 143' TERM
    mkdir -p "$WORK/db" "$WORK/cache" "$WORK/packages" "$WORK/root"
    # Eigene Paketdatenbank und eigener Cache; die System-Repositories bleiben unverändert.
    export PKG_DBDIR="$WORK/db" PKG_CACHEDIR="$WORK/cache"
    pkg_local() {
        "$PKG" -o "PKG_DBDIR=$WORK/db" -o "PKG_CACHEDIR=$WORK/cache" "$@"
    }
    printf '\nFreeBSD: %s / %s\n' "$(freebsd-version -u)" "$(uname -m)"
    printf 'Freier Speicher im Home-Verzeichnis:\n'
    df -h "$BASE"
    printf '\nLade Paketkatalog und Code-OSS samt Abhängigkeiten (kann mehrere GB benötigen).\n'
    pkg_local update || fail 'Paketkatalog nicht abrufbar. Netzwerk, Repository und FreeBSD-Version prüfen.'
    if ! pkg_local fetch -U -d -y -o "$WORK/packages" editors/vscode; then
        printf '\nAbhängigkeiten konnten nicht vollständig geladen werden. Versuche nur das Editorpaket.\n' >&2
        pkg_local fetch -U -y -o "$WORK/packages" editors/vscode || fail 'Kein passendes vscode-Paket im konfigurierten Repository. Bitte Administrator fragen.'
    fi
    # pkg kann Dateien entweder direkt oder in einem Hashed-Unterordner ablegen.
    find "$WORK/packages" -type f \( -name '*.pkg' -o -name '*.txz' \) -print > "$WORK/archives"
    if [ ! -s "$WORK/archives" ]; then
        find -L "$WORK/packages" -type f \( -name '*.pkg' -o -name '*.txz' \) -print > "$WORK/archives"
    fi
    [ -s "$WORK/archives" ] || fail 'Keine Paketarchive gefunden.'
    printf '\nEntpacke Pakete ausschließlich im persönlichen Verzeichnis.\n'
    while IFS= read -r archive; do
        printf '  %s\n' "$(basename "$archive")"
        tar -xf "$archive" -C "$WORK/root" --exclude '+*' || fail "Entpacken fehlgeschlagen: $archive"
    done < "$WORK/archives"
    LOCAL="$WORK/root/usr/local"
    BIN="$LOCAL/share/code-oss/code-oss"
    [ -x "$BIN" ] || fail 'Das erwartete native Code-OSS-Programm fehlt im Paket.'
    # Absolute Links aus dem Paket, sofern ihr Ziel ebenfalls lokal entpackt wurde.
    # Relative Links bleiben unverändert. Keine Links außerhalb der privaten Installation ändern.
    find "$LOCAL" -type l -print > "$WORK/links"
    while IFS= read -r link; do
        target=$(readlink "$link") || continue
        case "$target" in
            /usr/local/*)
                relative=${target#/usr/local/}
                if [ -e "$LOCAL/$relative" ]; then
                    # Relativer Link bleibt auch nach Verschieben des Staging-Verzeichnisses gültig.
                    parent=$(dirname "$link")
                    rest=${parent#"$LOCAL"}
                    back=''
                    while [ -n "$rest" ]; do
                        rest=${rest#/}
                        case "$rest" in */*) rest=${rest#*/} ;; *) rest='' ;; esac
                        back="../$back"
                    done
                    ln -snf "$back$relative" "$link"
                fi
                ;;
        esac
    done < "$WORK/links"
    printf '\nPrüfe native Bibliotheken.\n'
    if ! env LD_LIBRARY_PATH="$LOCAL/lib:$LOCAL/lib/compat${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" ldd "$BIN" > "$WORK/ldd.log" 2>&1; then
        cat "$WORK/ldd.log" >&2
        fail 'Das Programm ist auf diesem Rechner nicht ladbar (ABI oder Bibliotheken).'
    fi
    if grep -i 'not found' "$WORK/ldd.log"; then
        fail 'Fehlende Bibliotheken; eine vollständige lokale Installation ist hier nicht möglich.'
    fi
    [ ! -e "$BASE/root" ] || fail "Es gibt bereits eine unvollständige Installation unter $BASE/root. Bitte zuerst prüfen."
    mv "$WORK/root" "$BASE/root"
    printf '\nDateien installiert: %s\n' "$PREFIX"
    # Download/Staging aufräumen, nicht die Installation.
    rm -rf -- "$WORK"
    trap - EXIT INT TERM
fi
if [ "$INSTALL_ONLY" = yes ]; then
    printf 'Fertig vorbereitet. GUI noch nicht geprüft. Start: sh "%s/start-freebsd-vscode.sh" "%s"\n' "$HERE" "$PROJECT"
    exit 0
fi
printf '\nStarte Code-OSS mit Projekt: %s\n' "$PROJECT"
exec sh "$HERE/start-freebsd-vscode.sh" "$PROJECT"
