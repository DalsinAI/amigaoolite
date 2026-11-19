#!/bin/sh
# Read-only inventory of the Oolite build inputs on daletop, for a session
# that reaches daletop only through Cradle's stove_build (Team 5, 19 Nov).
# Changes nothing; writes $OUT/inventory.txt.
OUT=${OUT:-/tmp/oolite-probe-out}
mkdir -p "$OUT"
R="$OUT/inventory.txt"
{
echo "STOVE=$STOVE"; echo "HOME=$HOME"; date -u
for d in "$HOME/t5scratch/oolite-20261119" "$HOME/t5scratch" /tmp/atlas-batch10-20261117/stove-isolated/prefix \
         /tmp/gfopenfoundation-build-20261118 /tmp/gfopengnustep-20261118 \
         "$HOME/AmiCradle/stoves" "$HOME/WorkingSpace/AmigaChrome/team5" \
         "$HOME/DevDrive/AmiCradle-labs/oolite-rc1" /run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1; do
  echo "== $d"; ls -la "$d" 2>&1 | head -60
done
W="$HOME/t5scratch/oolite-20261119"
for d in "$W"/*/ ; do echo "== $d"; ls "$d" 2>&1 | head -40; done
echo "== git heads"
for g in "$W"/*/.git; do r=$(dirname "$g"); echo "$r $(git -C "$r" rev-parse --abbrev-ref HEAD 2>&1) $(git -C "$r" rev-parse --short HEAD 2>&1) $(git -C "$r" status --porcelain 2>/dev/null | wc -l) dirty"; done
echo "== built artefacts"
find "$W" -maxdepth 5 \( -name 'libgnustep-base.a' -o -name 'libSDL3.a' -o -name 'libGLU.a' -o -name 'libopenal.a' -o -name 'libvorbisfile.a' -o -name 'libffi.a' -o -name 'libjs_static.a' -o -name 'javascript.h' -o -name 'oolite-original-68040' -o -name 'libpthread-gfos.a' -o -name 'glu.h' -o -name 'SDL_video.h' -o -name 'al.h' \) 2>/dev/null | head -60
find /run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1 -maxdepth 4 \( -name 'libSDL3.a' -o -name 'libGLU.a' -o -name 'libvorbisfile.a' -o -name 'libffi.a' -o -name 'javascript.h' -o -name 'glu.h' \) 2>/dev/null | head -40
} > "$R" 2>&1
cat "$R" | tail -150
