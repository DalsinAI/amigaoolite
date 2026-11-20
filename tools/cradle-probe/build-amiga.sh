#!/bin/sh
# Read-only inventory of the Oolite build inputs on daletop, for a session
# that reaches daletop only through Cradle's stove_build (Team 5, 19 Nov).
# Changes nothing; prints a short report (the job log keeps its last lines).
OUT=${OUT:-/tmp/oolite-probe-out}; mkdir -p "$OUT"
W="$HOME/t5scratch/oolite-20261119"; RC=/run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1
echo "STOVE=$STOVE"; ls "$STOVE" 2>&1 | head; ls "$STOVE"/prefix/bin/m68k-amigaos-gcc "$STOVE"/bin/m68k-amigaos-gcc 2>&1
for p in /tmp/atlas-batch10-20261117/stove-isolated/prefix /tmp/gfopenfoundation-build-20261118 /tmp/gfopengnustep-20261118; do test -e "$p" && echo "EXISTS $p" || echo "GONE $p"; done
echo "== amigaoolite work tree status"; git -C "$W/amigaoolite" status --porcelain | head
echo "== out/"; ls "$W/amigaoolite/out" | head -40
echo "== out/gnustep-base-gfos"; ls "$W/amigaoolite/out/gnustep-base-gfos" | head
echo "== rc1 dependencies"; ls "$RC/dependencies" "$RC/dependencies"/* 2>&1 | head -60
echo "== rc1 source"; ls "$RC/source" "$RC/toolchain" "$RC/foundation" 2>&1 | head -40
echo "== libs"
for n in libgfosfoundationports.a libgfosfoundationipc.a libffi.a libopenal.a libGLU.a libSDL3.a libpng.a libz.a javascript.h jsl.h NSMessagePort.m.o; do
  echo "-- $n"; find "$W" "$RC" "$STOVE" -name "$n" 2>/dev/null | head -4
done
echo "== SDL3 headers"; find "$RC" "$W" -path '*SDL3/SDL_video.h' 2>/dev/null | head -3
echo "== GL headers"; find "$RC" "$W" "$STOVE" -name glu.h 2>/dev/null | head -3; find "$STOVE" -name 'libGL.a' 2>/dev/null | head -3
echo "== AL headers"; find "$RC" "$W" -path '*AL/al.h' 2>/dev/null | head -3
echo "== js sdk"; ls "$W/gfopensdk/Include" | head; find "$W/gfopensdk" -name 'javascript*' | head
echo "== done"
