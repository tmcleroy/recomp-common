#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#
# A port's icon in every form the systems want, from one SVG (1024 x 1024, its own rounded tile; docs/ICONS.md):
#
#   tools/make_icons.sh ICON.svg OUT [ANDROID_RES]
#
#   OUT/png/icon-N.png        16 24 32 48 64 128 256 512 1024 (Linux: hicolor/NxN/apps; the window's own: icon_png.h)
#   OUT/icon.icns             macOS (Contents/Resources, CFBundleIconFile), made on a Mac only (iconutil)
#   OUT/icon.ico              Windows (16-256 in one; a .rc's ICON line puts it in the .exe)
#   OUT/icon_png.h            the art without its tile, 256 on its longer side, as PNG bytes (kIconPng, kIconPngSize):
#                             the window's icon, the launcher's top bar, the in-game menu (icon.h)
#   ANDROID_RES/mipmap-*/ic_launcher.png, ic_launcher_round.png, ic_launcher_foreground.png (adaptive: the whole tile
#                             at 72 of 108 dp, over the tile's own colour: values/ic_launcher_background.xml)
#
# Needs ImageMagick 7 (magick). The SVG must use plain shapes (no transform="matrix(...)"): ImageMagick's own renderer
# places transformed shapes wrongly. The outputs are committed, so a build needs none of this.
set -eu
svg=$1 out=$2 res=${3:-}
command -v magick > /dev/null || { echo "make_icons.sh: needs ImageMagick (magick)" >&2; exit 1; }
if grep -q 'transform=' "$svg"; then
    echo "make_icons.sh: $svg has transform=: flatten it to plain shapes first (docs/ICONS.md)" >&2
    exit 1
fi
mkdir -p "$out/png"
render() { magick -background none -density 384 "$svg" -filter Lanczos -resize "$1x$1" -strip -depth 8 "PNG32:$2"; }
for n in 16 24 32 48 64 128 256 512 1024; do
    render "$n" "$out/png/icon-$n.png"
done
magick "$out/png/icon-16.png" "$out/png/icon-24.png" "$out/png/icon-32.png" "$out/png/icon-48.png" \
    "$out/png/icon-64.png" "$out/png/icon-128.png" "$out/png/icon-256.png" "$out/icon.ico"
if command -v iconutil > /dev/null; then
    set_=$(mktemp -d)/icon.iconset
    mkdir -p "$set_"
    for n in 16 32 128 256 512; do
        cp "$out/png/icon-$n.png" "$set_/icon_${n}x${n}.png"
        cp "$out/png/icon-$((n * 2)).png" "$set_/icon_${n}x${n}@2x.png" 2> /dev/null || render $((n * 2)) "$set_/icon_${n}x${n}@2x.png"
    done
    iconutil -c icns "$set_" -o "$out/icon.icns"
    rm -rf "$(dirname "$set_")"
fi
# (in the launcher and the menu the icon sits on their own dark ground: the art alone, without its tile (the
# rx="224" rect), cropped to it, 256 on its longer side)
mark=$(mktemp -d)
grep -v 'rx="224"' "$svg" > "$mark/mark.svg"
magick -background none -density 384 "$mark/mark.svg" -trim +repage -filter Lanczos -resize 256x256 -strip -depth 8 "PNG32:$mark/mark.png"
python3 - "$mark/mark.png" "$out/icon_png.h" <<'PY'
import sys
data = open(sys.argv[1], 'rb').read()
lines = [', '.join('0x%02x' % b for b in data[i:i + 16]) for i in range(0, len(data), 16)]
open(sys.argv[2], 'w').write(
    '// The icon as a 256 x 256 PNG (made by recomp-common tools/make_icons.sh from the icon SVG: do not edit)\n'
    '#pragma once\n#include <cstddef>\n\n'
    'inline constexpr unsigned char kIconPng[] = {\n    ' + ',\n    '.join(lines) + ',\n};\n'
    'inline constexpr size_t kIconPngSize = sizeof(kIconPng);\n')
PY
rm -rf "$mark"
if [ -n "$res" ]; then
    # (dp sizes: mdpi 1x .. xxxhdpi 4x; launcher icons 48 dp, adaptive layers 108 dp)
    for d in mdpi:1 hdpi:1.5 xhdpi:2 xxhdpi:3 xxxhdpi:4; do
        name=${d%%:*} k=${d#*:}
        mkdir -p "$res/mipmap-$name"
        legacy=$(python3 -c "print(round(48*$k))") layer=$(python3 -c "print(round(108*$k))") art=$(python3 -c "print(round(72*$k))")
        render "$legacy" "$res/mipmap-$name/ic_launcher.png"
        render "$legacy" "$res/mipmap-$name/ic_launcher_round.png"
        render "$art" /tmp/make_icons_art.png
        magick -size "${layer}x$layer" xc:none /tmp/make_icons_art.png -gravity center -composite -strip "$res/mipmap-$name/ic_launcher_foreground.png"
    done
    rm -f /tmp/make_icons_art.png
fi
echo "icons: $out"
