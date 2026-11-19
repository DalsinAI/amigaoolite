#!/bin/sh
# Compiles every upstream Oolite source that uses JSAPI against gfosjsapi
# (src/Core/gfosjsapi, no SpiderMonkey headers) and the GFOS Foundation, and
# reports errors per file. Usage: tools/gfosjsapi-compile-check.sh [file...]
# Team 5, 19 Nov 2026.
HERE="$(cd "$(dirname "$0")/.." && pwd)"
P=${PREFIX:-/tmp/atlas-batch10-20261117/stove-isolated/prefix}
U=${UPSTREAM:-$HERE/../oolite}
F=${GFOS_FOUNDATION_HEADERS:-$HERE/out/gnustep-base-gfos/Headers}
RTG=${GFOS_RTG:-/run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1/dependencies/gfopenrtg}
OUT="$HERE/out/jsapi-check"
mkdir -p "$OUT"
if [ $# -eq 0 ]; then
  set -- $(cd "$U/src" && grep -rlE '\bJS_[A-Z][A-Za-z]+\(|jsapi\.h|JSVAL_|jsval\b' --include=*.m --include=*.c . | sed 's#^\./##' | sort)
fi
total=0; clean=0
for f in "$@"; do
  total=$((total+1))
  obj="$OUT/$(echo "$f" | tr '/.' '__').o"
  log="$OUT/$(echo "$f" | tr '/.' '__').log"
  "$P/bin/m68k-amigaos-gcc" -x objective-c -std=gnu11 -O1 -m68040 -m68881 -noixemul \
    -fobjc-exceptions -fexceptions -D_NATIVE_OBJC_EXCEPTIONS -w \
    -DOOLITE_GFOS=1 -DOOLITE_BIG_ENDIAN=1 -DGL_GLEXT_PROTOTYPES=1 -DGNUSTEP_BASE_LIBRARY=1 -DNDEBUG \
    '-DOO_VERSION_FULL="1.93.1-255"' '-DOO_BUILDER="AmiCradle Team"' '-DOO_BUILD_DATE="2026-11-19"' \
    -include "$HERE/src/Core/gfos-amiga-tgmath-compat.h" \
    -I "$HERE/src/Core/gfosjsapi" -I "$HERE/src/Core" -I "$F" \
    -I "$U/src/Core" -I "$U/src/Core/Tables" -I "$U/src/Core/Entities" -I "$U/src/Core/Materials" \
    -I "$U/src/Core/Scripting" -I "$U/src/Core/Debug" -I "$U/src/Core/OXPVerifier" -I "$U/src/Core/MiniZip" \
    -I "$U/src/SDL" -I "$P/m68k-amigaos/ndk-include.vendor" -I "$P/m68k-amigaos/port-deps/include" \
    -I /tmp/oolite-rc1-sdl3-20261119/SDL3-3.4.14/include -I /tmp/oolite-rc1-glu-20261119/glu-9.0.3/include \
    -I "$RTG/build/include" \
    -c "$U/src/$f" -o "$obj" > "$log" 2>&1
  n=$(grep -c ' error: ' "$log")
  if [ "$n" -eq 0 ] && [ -s "$obj" ]; then clean=$((clean+1)); else printf '%4d %s\n' "$n" "$f"; fi
done
echo "clean: $clean of $total"
