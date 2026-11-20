#!/bin/sh
# Builds gfosjsapi (SpiderMonkey 1.8.5 API names on javascript.library) and its
# unit gate: out/gfosjsapi/gfosjsapi-gate. Run it on GFOS with a 1 MB stack;
# evidence in tests/evidence/gfosjsapi-gate-*.log. Team 5, 19 Nov 2026.
set -e
HERE="$(cd "$(dirname "$0")/.." && pwd)"
P=${PREFIX:-/tmp/atlas-batch10-20261117/stove-isolated/prefix}
SDK=${GFOPENSDK:-$HERE/../gfopensdk}
OUT="$HERE/out/gfosjsapi"
mkdir -p "$OUT"
CC="$P/bin/m68k-amigaos-gcc -std=gnu11 -O2 -m68040 -m68881 -noixemul -Wall -Wno-unused-function \
 -I $HERE/src/Core/gfosjsapi -I $SDK/Include -I $P/m68k-amigaos/ndk-include.vendor"
$CC -c "$HERE/src/Core/gfosjsapi/oojsl.c" -o "$OUT/oojsl.o"
$CC -c "$HERE/src/Core/gfosjsapi/gfosjsapi.c" -o "$OUT/gfosjsapi.o"
$CC -c "$HERE/tests/gfosjsapi_gate.c" -o "$OUT/gate.o"
$CC -c "$HERE/tests/gfosjsapi_gate_main.c" -o "$OUT/gate_main.o"
"$P/bin/m68k-amigaos-gcc" -m68040 -m68881 -noixemul "$OUT/gate_main.o" "$OUT/gate.o" "$OUT/gfosjsapi.o" "$OUT/oojsl.o" \
  -lm -o "$OUT/gfosjsapi-gate"
echo "built $OUT/gfosjsapi-gate"
