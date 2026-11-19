#!/bin/sh
# Stages Oolite's upstream Resources for a lab: out/oolite-resources.tar and
# out/ootar (a ustar unpacker for the Amiga). lab_put both, then in the lab:
#   cd SYS:OoliteGame ; ootar oolite-resources.tar
# Team 5, 19 Nov 2026.
set -e
HERE="$(cd "$(dirname "$0")/../.." && pwd)"
OUT=${OUT:-$HERE/out/stage}
W=${OOLITE_WORK:-$HOME/t5scratch/oolite-20261119}
U=${UPSTREAM:-$W/oolite}
P=${PREFIX:-/tmp/atlas-batch10-20261117/stove-isolated/prefix}
mkdir -p "$OUT"
test "$(git -C "$U" rev-parse HEAD)" = 21978c37b2f11c8664adf3e2062efc1d0c875979
"$P/bin/m68k-amigaos-gcc" -std=gnu11 -O2 -m68040 -noixemul -Wall -o "$OUT/ootar" "$HERE/tools/cradle-stage/ootar.c"
tar --format=ustar -C "$U" -cf "$OUT/oolite-resources.tar" Resources
echo "resources: $(tar -tf "$OUT/oolite-resources.tar" | wc -l) entries, $(stat -c %s "$OUT/oolite-resources.tar") bytes"
