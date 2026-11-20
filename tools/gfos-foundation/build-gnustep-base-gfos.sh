#!/bin/sh
# Rebuilds GNUstep Base 1.31.1 for GFOS the way Oolite (and any GFOS
# Objective-C program) needs it:
#   - 68040 with its FPU (-m68040 -m68881): the exploratory build was 68000
#     soft-float, so every double crossing into Foundation was garbage;
#   - native Objective-C exceptions (-fexceptions -fobjc-exceptions
#     -D_NATIVE_OBJC_EXCEPTIONS, BASE_NATIVE_OBJC_EXCEPTIONS 1): the
#     exploratory build used setjmp exceptions, which a program's @catch
#     never sees;
#   - AmigaOS path roots ("SYS:", "PROGDIR:"): patches/gnustep/0002.
#
# Input: a configured GNUstep Base tree with the GFOS source edits (today the
# exploratory tree from GFOpenGNUstep's notes). It is copied, never changed.
# Output: $OUT/libgnustep-base.a and $OUT/Headers (use these headers: the
# exception mode is in GSConfig.h).
#
# Until this lives in GFOpenGNUstep (and the SDK), this script is how
# amigaoolite builds its Foundation. Team 5, 19 Nov 2026.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
SRC_TREE=${SRC_TREE:-/tmp/gfopenfoundation-build-20261118/gnustep-libs-base-b1bfc5b}
HOST_GNUSTEP=${HOST_GNUSTEP:-/tmp/gfopenfoundation-build-20261118/host-gnustep}
FFI=${FFI:-/tmp/gfopenfoundation-build-20261118/target-libffi}
P=${PREFIX:-/tmp/atlas-batch10-20261117/stove-isolated/prefix}
OUT=${OUT:-$REPO/out/gnustep-base-gfos}
WORK="$OUT/tree"

rm -rf "$OUT"; mkdir -p "$OUT"
cp -a "$SRC_TREE" "$WORK"
cd "$WORK"
patch -s -p1 < "$REPO/patches/gnustep/0002-gfos-amiga-path-roots.patch"
for h in Headers/GNUstepBase/GSConfig.h Source/GNUstepBase/GSConfig.h; do
  [ -f "$h" ] && sed -i 's/^#define BASE_NATIVE_OBJC_EXCEPTIONS     0/#define BASE_NATIVE_OBJC_EXCEPTIONS     1/' "$h"
done
grep -q 'BASE_NATIVE_OBJC_EXCEPTIONS     1' Headers/GNUstepBase/GSConfig.h

export PATH="$HOST_GNUSTEP/bin:$PATH"
export GNUSTEP_MAKEFILES="$HOST_GNUSTEP/share/GNUstep/Makefiles"
export CPATH="$P/m68k-amigaos/ndk-include.vendor"
FLAGS="-m68040 -m68881 -fexceptions -fobjc-exceptions -D_NATIVE_OBJC_EXCEPTIONS"
cd Source
find . -name '*.o' -delete
# -k: NSMessagePortNameServer.m does not build for GFOS; the exploratory
# archive leaves it out and Oolite's link compiles a patched copy.
make -k CC="$P/bin/m68k-amigaos-gcc" OBJC="$P/bin/m68k-amigaos-gcc" CXX="$P/bin/m68k-amigaos-g++" \
     shared=no messages=no -j8 \
     ADDITIONAL_CFLAGS="$FLAGS" ADDITIONAL_OBJCFLAGS="$FLAGS" \
     > "$OUT/build.log" 2>&1 || true
# GSFFIInvocation.m (WITH_FFI=libffi) is not reached by the -k run.
[ -f obj/libgnustep-base.obj/GSFFIInvocation.m.o ] || \
"$P/bin/m68k-amigaos-gcc" GSFFIInvocation.m -c '-DGNUSTEP_TARGET_DIR="."' '-DGNUSTEP_TARGET_CPU="x86_64"' \
  '-DGNUSTEP_TARGET_OS="linux-gnu"' '-DGNUSTEP_IS_FLATTENED="yes"' '-DLIBRARY_COMBO="gnu-gnu-gnu"' \
  -DGNUSTEP_BASE_INTERNAL=1 -DGNUSTEP -DGNUSTEP_BASE_LIBRARY=1 -DGNU_RUNTIME=1 -fno-strict-aliasing \
  -DGSWARN -DGSDIAGNOSE -Wno-import -g -O2 $FLAGS -I../Headers -I. \
  -I"$HOST_GNUSTEP/include" -I"$FFI/include" -o obj/libgnustep-base.obj/GSFFIInvocation.m.o
# The archive: exactly the members of the exploratory archive.
ORIG="$SRC_TREE/Source/obj/libgnustep-base.a"
for m in $("$P/bin/m68k-amigaos-ar" t "$ORIG"); do
  f=$(find . -path '*/obj/*' -name "$m" | head -1)
  [ -n "$f" ] || { echo "MISSING $m" >&2; exit 1; }
  echo "$f"
done > "$OUT/members.txt"
xargs "$P/bin/m68k-amigaos-ar" rcs "$OUT/libgnustep-base.a" < "$OUT/members.txt"
cp -a ../Headers "$OUT/Headers"
echo "$OUT/libgnustep-base.a: $("$P/bin/m68k-amigaos-ar" t "$OUT/libgnustep-base.a" | wc -l) members"
