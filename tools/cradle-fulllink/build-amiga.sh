#!/bin/sh
# Full Oolite link with NO SpiderMonkey: the 242 upstream sources against
# gfosjsapi (src/Core/gfosjsapi), the GFOS Foundation of PR #4, and the game on
# javascript.library's script task (src/Core/OOGFOSGameOnScriptTask.c, patch
# 0016). Run by Cradle's stove_build on daletop (STOVE= and OUT= set); also by
# hand: OUT=dir sh tools/cradle-fulllink/build-amiga.sh.
#
# Inputs are daletop's, by path, until the pinned SDK inputs of Dale's
# decision 4 exist (team5 work tree for the upstream checkout and the
# Foundation; the oolite-rc1 lab folder for SDL3, OpenAL, GLU, ogg/vorbis and
# libffi). Objects are cached by content in $CACHE, so a rerun compiles only
# what changed.
#
# Writes to $OUT: oolite-68040 (when it links), link.log, undefined.txt (one
# line per missing symbol, with how many references), compile-errors.txt,
# summary.txt. Team 5, 19 Nov 2026.
set -u
HERE="$(cd "$(dirname "$0")/../.." && pwd)"
OUT=${OUT:-$HERE/out/fulllink}
W=${OOLITE_WORK:-$HOME/t5scratch/oolite-20261119}
RC=${OOLITE_RC:-/run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1}
P=${PREFIX:-/tmp/atlas-batch10-20261117/stove-isolated/prefix}
U=${UPSTREAM:-$W/oolite}
FD=${GFOS_FOUNDATION_DIR:-$W/amigaoolite/out/gnustep-base-gfos}
PTH=${GFOS_PTHREAD_LIB:-$W/amigaoolite/out/libpthread-gfos.a}
GLIB=${GFOS_LIB_DIR:-$RC/foundation/gfopengnustep/ci-check/out}
FFI=${GFOS_FFI_LIB:-$RC/foundation/gfopenfoundation-build/target-libffi/lib/libffi.a}
RTG=${GFOS_RTG:-$RC/dependencies/gfopenrtg}
OGG=${OGG_ROOT:-$RC/dependencies/legacy-debug/ogg-vorbis-stage}
SDK=${GFOPENSDK:-$W/gfopensdk}
# SDL3 with OpenGL (gfopenrtg team5/sdl3-opengl-20261119, SDL3.module 3): that
# branch's Cradle build until it is merged and in the pinned SDK inputs.
SDL3D=${GFOS_SDL3_DIR:-/run/media/da1ek/DevDrive/AmiCradle-dev/cradle-builds/jobs/j20261119T231525-fac24e/out}
test -r "$SDL3D/libSDL3.a" || SDL3D=$RTG/build
CACHE=${CACHE:-$W/fulllink-cache}
JOBS=${JOBS:-$(nproc 2>/dev/null || echo 4)}
CC="$P/bin/m68k-amigaos-gcc"
CXX="$P/bin/m68k-amigaos-g++"

mkdir -p "$OUT" "$CACHE"
SUM="$OUT/summary.txt"
: > "$SUM"
say() { echo "$*"; echo "$*" >> "$SUM"; }

# -- inputs
missing=0
for f in "$CC" "$U/src/SDL/main.m" "$FD/libgnustep-base.a" "$FD/Headers/Foundation/Foundation.h" "$PTH" \
         "$GLIB/libgfosfoundationports.a" "$GLIB/libgfosfoundationipc.a" "$FFI" "$SDL3D/libSDL3.a" \
         "$SDL3D/include/SDL3/SDL_video.h" "$RTG/build/libopenal.a" "$RTG/build/include/AL/al.h" \
         "$RTG/build/glu/lib/libGLU.a" "$RTG/build/glu/include/GL/glu.h" "$OGG/lib/libvorbisfile.a" \
         "$SDK/Include/proto/javascript.h"; do
  test -e "$f" || { say "MISSING $f"; missing=1; }
done
test "$(git -C "$U" rev-parse HEAD 2>/dev/null)" = 21978c37b2f11c8664adf3e2062efc1d0c875979 || { say "MISSING upstream at 21978c37 in $U"; missing=1; }
PD=""
for d in "$P/m68k-amigaos/port-deps/lib" "$RC/toolchain/prefix/m68k-amigaos/port-deps/lib"; do
  test -r "$d/libpng.a" && test -r "$d/libz.a" && { PD="$d"; break; }
done
test -n "$PD" || { say "MISSING port-deps libpng.a/libz.a"; missing=1; }
GLLIB=$(find "$RTG/build" "$RTG/sdk" "$P/m68k-amigaos/lib" -name libGL.a 2>/dev/null | head -1)
test -n "$GLLIB" || { say "MISSING libGL.a"; missing=1; }
test $missing -eq 0 || { say "inputs missing: stopping"; exit 1; }
say "inputs: upstream 21978c37; foundation $FD; port-deps $PD; libGL $GLLIB; SDL3 $SDL3D; jobs $JOBS"

# -- patched upstream copies (never the checkout itself)
PS="$OUT/patched-src"
rm -rf "$PS"; mkdir -p "$PS/src"
for name in Core/GameController.m Core/OOLogOutputHandler.m Core/OOLogging.m Core/Universe.m Core/OOCPUInfo.m \
            Core/OOCPUInfo.h Core/OOLogHeader.m Core/OOOpenGL.m Core/OOPolygonSprite.m SDL/EXRSnapshotSupport/miniz.c \
            SDL/main.m SDL/OOSDLJoystickManager.m SDL/MyOpenGLView.m Core/Scripting/OOJSScript.m; do
  mkdir -p "$PS/src/$(dirname "$name")"; cp "$U/src/$name" "$PS/src/$name"
done
for p in 0001-gfos-universe-frame-pump 0002-gfos-native-cpu-memory-report 0003-gfos-native-glu-header \
         0004-gfos-polygon-glu 0005-gfos-miniz-stdio 0006-gfos-gnustep-process-bootstrap \
         0008-gfos-native-synchronous-logging 0010-gfos-defer-complex-log-header \
         0011-gfos-defer-early-log-preferences 0013-gfos-controller-default-tick \
         0014-gfos-sdl3-joystick-lifecycle 0015-gfos-view-early-prefs-fallback 0016-gfos-main-on-script-task \
         0017-gfos-no-xdr-script-cache 0018-gfos-launch-trace \
         0019-gfos-game-loop-without-runloop; do
  (cd "$PS" && patch -s -N -p1 < "$HERE/patches/oolite/$p.patch") || { say "PATCH FAILED $p"; exit 1; }
done

# (single-quoted inside: DEFS is written into cc1.sh as text)
DEFS="-DOOLITE_GFOS=1 -DOOLITE_BIG_ENDIAN=1 -DGL_GLEXT_PROTOTYPES=1 -DGNUSTEP_BASE_LIBRARY=1 -DNDEBUG -DOOLITE_GFOS_STARTUP_TRACE=1 \
 -DUSE_FILE32API=1 -DOO_VERSION_FULL='\"1.93.1-255\"' -DOO_BUILDER='\"AmiCradle Team\"' -DOO_BUILD_DATE='\"2026-11-19\"'"
INCS="-I $HERE/src/Core/gfosjsapi -I $PS/src/Core -I $HERE/src/Core -I $FD/Headers \
 -I $U/src/Core -I $U/src/Core/Tables -I $U/src/Core/Entities -I $U/src/Core/Materials -I $U/src/Core/Scripting \
 -I $U/src/Core/Debug -I $U/src/Core/OXPVerifier -I $U/src/Core/MiniZip -I $U/src/SDL -I $U/src/SDL/EXRSnapshotSupport \
 -I $P/m68k-amigaos/ndk-include.vendor -I $P/m68k-amigaos/include -I $P/m68k-amigaos/port-deps/include \
 -I $SDL3D/include -I $RTG/build/include -I $RTG/build/glu/include -I $OGG/include"
COMMON="-O2 -m68040 -m68881 -noixemul -fexceptions -D_NATIVE_OBJC_EXCEPTIONS -w $DEFS"

# -- one compile, cached by (compiler, flags, source, every header it pulled in is not tracked:
#    the key covers the source, the patches, gfosjsapi's headers and the flags)
KEYSALT=$( { cat "$HERE/src/Core/gfosjsapi/"*.h "$HERE/src/Core/gfos-amiga-tgmath-compat.h"; echo "$COMMON $INCS"; "$CC" --version | head -1; } | sha1sum | cut -c1-12)
cat > "$OUT/cc1.sh" <<EOF
#!/bin/sh
name="\$1"; src="$U/src/\$name"; test -r "$PS/src/\$name" && src="$PS/src/\$name"
key=\$( { cat "\$src"; echo "\$name $KEYSALT"; } | sha1sum | cut -c1-16)
obj="$CACHE/\$key.o"; log="$OUT/logs/\$(echo "\$name" | tr '/.' '__').log"
test -s "\$obj" && { echo "\$obj"; exit 0; }
case "\$name" in
  *.m) set -- "$CC" -x objective-c -std=gnu11 -fobjc-exceptions -include "$HERE/src/Core/gfos-amiga-tgmath-compat.h";;
  *.c) set -- "$CC" -x c -std=gnu11 -include "$HERE/src/Core/gfos-amiga-tgmath-compat.h";;
  *.cpp) set -- "$CXX" -x c++ -std=gnu++17;;
esac
"\$@" $COMMON $INCS -c "\$src" -o "\$obj.tmp" > "\$log" 2>&1 && mv "\$obj.tmp" "\$obj" && echo "\$obj" && exit 0
rm -f "\$obj.tmp"; echo "ERROR \$name" >&2; exit 1
EOF
mkdir -p "$OUT/logs"
t0=$(date +%s)
grep -v '^$' "$HERE/build/oolite-meson-242.list" | xargs -P "$JOBS" -n 1 sh "$OUT/cc1.sh" > "$OUT/objects.txt" 2> "$OUT/compile-errors.txt"
nobj=$(grep -c '\.o$' "$OUT/objects.txt"); nerr=$(grep -c '^ERROR' "$OUT/compile-errors.txt")
say "upstream: $nobj objects, $nerr failed ($(( $(date +%s) - t0 )) s)"
if [ "$nerr" -gt 0 ]; then
  for n in $(sed -n 's/^ERROR //p' "$OUT/compile-errors.txt"); do
    l="$OUT/logs/$(echo "$n" | tr '/.' '__').log"; say "--- $n"; grep -m 5 'error:' "$l" | sed 's/^/    /' | tee -a "$SUM"
  done
fi

# -- GFOS pieces
G="$OUT/gfos"; mkdir -p "$G"
gc() { out="$1"; shift; "$@" > "$G/$out.log" 2>&1 || { say "GFOS COMPILE FAILED $out"; sed -n 1,20p "$G/$out.log" | tee -a "$SUM"; return 1; }; }
GC="$CC -std=gnu11 -O2 -m68040 -m68881 -noixemul -Wall -Wno-unused-function"
gc oojsl $GC -I "$HERE/src/Core/gfosjsapi" -I "$SDK/Include" -I "$P/m68k-amigaos/ndk-include.vendor" -c "$HERE/src/Core/gfosjsapi/oojsl.c" -o "$G/oojsl.o"
gc gfosjsapi $GC -I "$HERE/src/Core/gfosjsapi" -I "$P/m68k-amigaos/ndk-include.vendor" -c "$HERE/src/Core/gfosjsapi/gfosjsapi.c" -o "$G/gfosjsapi.o"
gc onscript $GC -I "$HERE/src/Core" -I "$SDK/Include" -I "$P/m68k-amigaos/ndk-include.vendor" -c "$HERE/src/Core/OOGFOSGameOnScriptTask.c" -o "$G/OOGFOSGameOnScriptTask.o"
gc process $GC -I "$HERE/src/Core" -I "$P/m68k-amigaos/ndk-include.vendor" -c "$HERE/src/Core/OOGFOSProcess.c" -o "$G/OOGFOSProcess.o"
gc passwd $GC -idirafter "$P/m68k-amigaos/ndk-include.vendor" -c "$HERE/src/Core/OOGFOSPasswd.c" -o "$G/OOGFOSPasswd.o"
gc sysinfo $GC -I "$HERE/src/Core" -I "$P/m68k-amigaos/ndk-include.vendor" -c "$HERE/src/Core/OOGFOSSystemInfo.c" -o "$G/OOGFOSSystemInfo.o"
gc preflight $GC -I "$HERE/src/Core" -I "$U/src/Core/MiniZip" -I "$P/m68k-amigaos/port-deps/include" -c "$HERE/src/Core/OOOXZArchivePreflight.c" -o "$G/OOOXZArchivePreflight.o"
gc extractor "$CC" -x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul -fobjc-exceptions -fexceptions -D_NATIVE_OBJC_EXCEPTIONS -w \
   -I "$HERE/src/Core" -I "$FD/Headers" -I "$U/src/Core/MiniZip" -I "$P/m68k-amigaos/port-deps/include" -c "$HERE/src/Core/OOOXZGFOSExtraction.m" -o "$G/OOOXZGFOSExtraction.o"

# -- link: no libjs_static.a anywhere
EXE="$OUT/oolite-68040"
rm -f "$EXE"
"$CXX" -fexceptions -m68040 -m68881 -noixemul -Wl,--wrap=exit -Wl,--wrap=getcwd -Wl,-Map,"$OUT/oolite.map" \
  $(cat "$OUT/objects.txt") "$G"/*.o \
  -L "$PD" -L "$P/m68k-amigaos/libnix/lib" \
  -Wl,--start-group -Wl,--whole-archive "$FD/libgnustep-base.a" -Wl,--no-whole-archive \
  "$GLIB/libgfosfoundationports.a" "$GLIB/libgfosfoundationipc.a" \
  "$SDL3D/libSDL3.a" "$RTG/build/libopenal.a" "$RTG/build/glu/lib/libGLU.a" \
  "$OGG/lib/libvorbisfile.a" "$OGG/lib/libvorbis.a" "$OGG/lib/libogg.a" "$PD/libpng.a" "$GLLIB" \
  -lobjc "$FFI" "$PD/libz.a" "$PTH" -lsocket -lm -Wl,--end-group -o "$EXE" > "$OUT/link.log" 2>&1
lrc=$?
grep -o "undefined reference to \`[^']*'" "$OUT/link.log" | sed "s/undefined reference to \`//;s/'$//" | sort | uniq -c | sort -rn > "$OUT/undefined.txt"
grep -o "multiple definition of \`[^']*'" "$OUT/link.log" | sort -u > "$OUT/multiple.txt"
if [ $lrc -eq 0 ] && [ -s "$EXE" ]; then
  say "LINKED $EXE ($(stat -c %s "$EXE") bytes)"
  "$P/bin/m68k-amigaos-nm" "$EXE" 2>/dev/null | grep -ci ' js_\|mozjs\|spidermonkey' | sed 's/^/spidermonkey-ish symbols (gfosjsapi JS_ names expected): /' | tee -a "$SUM"
else
  say "LINK FAILED (rc $lrc): $(wc -l < "$OUT/undefined.txt") undefined symbols, $(wc -l < "$OUT/multiple.txt" 2>/dev/null || echo 0) multiple definitions"
  head -80 "$OUT/undefined.txt" | tee -a "$SUM"
  test -s "$OUT/multiple.txt" && head -20 "$OUT/multiple.txt" | tee -a "$SUM"
  grep -v "undefined reference\|multiple definition\|more undefined\|first defined\|In function\|^/.*: in function" "$OUT/link.log" | head -30 | tee -a "$SUM"
fi
rm -rf "$PS"
find "$OUT/logs" "$OUT/gfos" -name "*.log" -size 0 -delete 2>/dev/null
exit 0
