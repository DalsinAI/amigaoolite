#!/bin/sh
# FPU float -> integer probe (tests/fpprobe.c), for Cradle's stove_build: out/fpprobe.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
OUT=${OUT:?}; : "${STOVE:?}"
CC=$(ls "$STOVE"/prefix/bin/m68k-amigaos-gcc "$STOVE"/bin/m68k-amigaos-gcc 2>/dev/null | head -1)
"$CC" -O2 -m68040 -m68881 -noixemul -o "$OUT/fpprobe" "$HERE/tests/fpprobe.c" -lm
ls -l "$OUT/fpprobe"
# the assembly, so a wrong line can be traced to its instruction
"$CC" -O2 -m68040 -m68881 -noixemul -S -o "$OUT/fpprobe.s" "$HERE/tests/fpprobe.c"
grep -n "fintrz\|fmove.l\|fmove.w\|fmove.b\|fdmove\|fsmove\|jsr\|bsr" "$OUT/fpprobe.s" | head -60
