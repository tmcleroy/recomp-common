#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of recomp-common: https://github.com/tmcleroy/recomp-common
#
# Checks the repository's tracked files for things that must never be committed (CONTRIBUTING.md):
# game files, keys, personal paths and addresses, large files; and that every source file says what
# its license is. Run it before pushing (make audit); CI runs it on every pull request. Exits
# non-zero on a finding.
set -eu
cd "$(git rev-parse --show-toplevel)"
LIST="$(mktemp)"
trap 'rm -f "$LIST"' EXIT
git ls-files > "$LIST"

fail=0
check() { if [ -n "$2" ]; then echo "AUDIT FAIL: $1"; echo "$2" | head -10; fail=1; else echo "ok: $1"; fi; }
# (this script names the things it looks for, so it is not searched itself)
grep_files() { grep -v '^tools/audit\.sh$' "$LIST" | tr '\n' '\0' | xargs -0 grep -nIE "$@" 2>/dev/null || true; }

check "no game or archive file types" "$(grep -iE '\.(smc|sfc|nes|gb|gbc|gba|n64|z64|gen|bin|cue|iso|chd|gdi|wad|dol|rel|nds|3ds|xbe|xiso|zip|7z|rar|key|sav|srm|state|eep|rom)$' "$LIST" || true)"
check "no pictures but the two of the menu itself" "$(grep -iE '\.(png|jpe?g|gif|bmp|ppm|webp)$' "$LIST" | grep -vE '^docs/menu(-page)?\.png$' || true)"
check "no key-like hex strings (32 digits or more)" "$(grep_files '\b[0-9a-fA-F]{32,}\b' || true)"
check "no home folder paths" "$(grep_files -e '/Users/' -e '/home/[a-z]' -e 'C:\\\\Users' || true)"
check "no private session links or private network addresses" "$(grep_files -e 'claude\.ai/code/session' -e '\b192\.168\.[0-9]+\.[0-9]+' -e '\b10\.[0-9]+\.[0-9]+\.[0-9]+\b' || true)"
check "no file over 256 KB" "$(tr '\n' '\0' < "$LIST" | xargs -0 find 2>/dev/null -prune -type f -size +256k || true)"
check "every source file has a license line" "$(grep -E '\.(h|c|cpp|sh)$' "$LIST" | tr '\n' '\0' | xargs -0 grep -L 'SPDX-License-Identifier: GPL-3.0-only' 2>/dev/null || true)"
[ "$fail" = 0 ] || { echo "audit failed"; exit 1; }
echo "audit passed ($(wc -l < "$LIST" | tr -d ' ') files)"
