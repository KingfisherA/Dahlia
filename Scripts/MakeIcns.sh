#!/bin/bash
# Regenerates Icons/Dahlia.icns from Icons/Dahlia Icon.svg (macOS only).
# Needs Inkscape (/Applications/Inkscape.app or on PATH) or rsvg-convert (brew install librsvg);
# both keep the SVG's filters and transparency.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SVG="$ROOT/Icons/Dahlia Icon.svg"
ICNS="$ROOT/Icons/Dahlia.icns"

if command -v inkscape >/dev/null 2>&1; then
    INKSCAPE=inkscape
elif [ -x /Applications/Inkscape.app/Contents/MacOS/inkscape ]; then
    INKSCAPE=/Applications/Inkscape.app/Contents/MacOS/inkscape
else
    INKSCAPE=
fi

render() { # <size> <output.png>
    if [ -n "$INKSCAPE" ]; then
        "$INKSCAPE" "$SVG" --export-type=png --export-background-opacity=0 \
            --export-width="$1" --export-height="$1" --export-filename="$2" 2>/dev/null
    elif command -v rsvg-convert >/dev/null 2>&1; then
        rsvg-convert -w "$1" -h "$1" -o "$2" "$SVG"
    else
        echo "Install Inkscape or librsvg (brew install librsvg) to render the icon" >&2
        exit 1
    fi
}

ICONSET="$(mktemp -d)/Dahlia.iconset"
mkdir -p "$ICONSET"
trap 'rm -rf "$(dirname "$ICONSET")"' EXIT

for SIZE in 16 32 128 256 512; do
    render "$SIZE" "$ICONSET/icon_${SIZE}x${SIZE}.png"
    render "$((SIZE * 2))" "$ICONSET/icon_${SIZE}x${SIZE}@2x.png"
done

iconutil --convert icns --output "$ICNS" "$ICONSET"
echo "Wrote $ICNS"
