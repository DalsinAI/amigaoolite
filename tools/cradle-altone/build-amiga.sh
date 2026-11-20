#!/bin/sh
# OpenAL tone probe (tests/altone.c), for Cradle's stove_build: out/altone, linked against the same
# libopenal.a as the game (tools/cradle-fulllink). MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
OUT=${OUT:?}; : "${STOVE:?}"
RC=${OOLITE_RC:-/run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1}
RTG=${GFOS_RTG:-$RC/dependencies/gfopenrtg}
CC=$(ls "$STOVE"/prefix/bin/m68k-amigaos-gcc "$STOVE"/bin/m68k-amigaos-gcc 2>/dev/null | head -1)
"$CC" -O2 -m68040 -m68881 -noixemul -std=gnu11 -I "$RTG/build/include" -o "$OUT/altone" "$HERE/tests/altone.c" \
  "$RTG/build/libopenal.a" -lm
ls -l "$OUT/altone"
