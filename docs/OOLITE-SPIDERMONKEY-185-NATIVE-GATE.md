# Oolite native 68040 SpiderMonkey compatibility gate

18 November 2026, verified against pinned OoliteProject/oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`. This is a **verified build dependency handoff**, not a working JavaScript engine port.

## Exact authoritative source

Oolite's own `Doc/SpidermonkeyChanges.txt` identifies SpiderMonkey **1.8.5 / Mozilla Firefox 4.0** and Oolite-specific JS macro needs. Its `ShellScripts/macOS/build_mozilla_js.sh` pins `OoliteProject/mozillajs-linux` commit `bc12f7124166c199925f3cea37d2732d3704213d`. That repository and exact commit were fetched and verified on daletop. The native Linux Meson build expects `libjs_static.a` (or `libjsdbg_static.a` for debug) and headers under `build/mozilla_js/include`, with `nspr` as a separate dependency. The source's `libjs.make` uses `--disable-shared-js --enable-threadsafe --with-system-nspr --disable-tests --enable-trace-jscalls` and expects Python 2.7. A pre-existing validated target 68040 SpiderMonkey library, its generated `js-config.h`, target NSPR or Python 2 interpreter was **not found** in the inspected isolated toolchain; this is not proof those tools don't exist elsewhere.

## Why this is the current full OXZ manager compile blocker

The actual patched upstream `src/Core/OOOXZManager.m` was cross-compiled against the full pinned original Oolite tree using GCC 16.2 `-m68040 -m68881 -noixemul`. The original first error `SDL3/SDL_opengl.h` was addressed with existing `patches/0001-gfos-native-GL-geometry.patch` using GFOpenGPU headers. The source's `OOViewID.tbl` include lives in `src/Core/Tables`. `OOMaths.h` originally expanded `tgmath.h` to unavailable long-double complex `csinl`/`ccosl`; native `patches/0009-gfos-oomaths-tgmath.patch` was independently tested with actual Oolite quaternion/matrix code in a 68040 guest, exit 0. The next full-class error is `GuiDisplayGen.h: #include <jsapi.h>`. Oolite uses the old SpiderMonkey 1.8.5 JSAPI; do **not** satisfy this with a modern, incompatible `jsapi.h` or a fabricated stub.

## Next code and test milestones

1. Produce genuine m68k-compatible NSPR headers/library and check their thread/synchronization assumptions against GFOS/OpenMulticore. Do not implement an alternate SDK.
2. Cross-compile the pinned SpiderMonkey 1.8.5 **interpreter-only** using supported 32-bit big-endian configuration, disabling unsupported JIT paths, against the existing GCC16 native target and validated NSPR. Generate real target `js-config.h`, not an invented host-width header.
3. Link and guest-run a tiny real JSAPI test (JS_NewRuntime/context/global, evaluate a deterministic expression, teardown) under native AmigaOS and confirm no task leaks.
4. Rebuild `OOOXZManager.m` using genuine headers and the existing GFOpenGPU/OOFoundation ports, then resolve remaining actual engine dependencies. No release/full game claim before startup, rendering and input are shown in the guest.

The upstream source was staged for inspection under daletop's `/dev/shm/oolite-oxz/mozillajs-linux` with no local Git commits. This document does not vendor or relicense the Mozilla source. No Team 4 multicore SDK changes were made.
