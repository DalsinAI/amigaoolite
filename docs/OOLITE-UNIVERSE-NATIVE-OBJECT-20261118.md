# Original Oolite Universe.m native 68040 object: first compile gate

18 November 2026, 14:46–14:58 UTC. Source repository: DalsinAI/amigaoolite. Pinned Oolite original tree: `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`. The pinned workspace already carried previous GFOS source patches (notably OOOpenGLOnly.h and OOMaths.h); they are **prerequisites**, not pristine upstream. We did not modify the checked-out vendor tree in this qualification.

## What actually built

A copy of the original `src/Core/Universe.m` was made in isolated scratch and patched with this repo's `patches/oolite/0001-gfos-universe-frame-pump.patch`, enabling `OOGFOSGameFrameUpdate` inside `-[Universe update:]` under `OOLITE_GFOS_GAME_DISPATCH`. GCC 16.2 m68k cross-compiled **that entire original 10,000-line Objective-C class** into a native AmigaOS 68040 object at `-O2 -m68040 -m68881 -noixemul`, `-fobjc-exceptions`, `-DNDEBUG`, `-DOOLITE_BIG_ENDIAN=1`, `-DGL_GLEXT_PROTOTYPES=1`. **Actual `-c` succeeds with RC0.**

Reproducible Makefile target: `make universe-gfos-object UPSTREAM=<pinned-oolite-tree> PREFIX=<gcc16-stove> GFOS_JS_SDK=<gfopensdk> GFOS_SDL3_HEADERS=<gfopenrtg-sdl3-build/include> GFOS_OPENAL_HEADERS=<gfopenrtg/library/modules/openal/include> GFOS_JS185_HEADERS=<genuine-m68k-mozjs185/dist/include> OUT=<scratch>`. The target refuses upstream commit mismatch and checks the supplied genuine SDL3/OpenAL/SpiderMonkey headers, copies and patches only into OUT. Running `make` then `make -B` generated identical 68040 object SHA-256 `09a3c101892ee54b012a3f368b3f53b87cb75afb16c8229753f4bf71c3950e2c`, size approximately 536 KiB. A separately authored direct compile also succeeded (different object hash/size because include paths and source location differed); no false bit-identical claim across different compilation inputs.

## Canonical inputs — not fabricated headers

- Oolite pinned original Objective-C source and existing GFOS native patches, `GFOpenGNUstep` Foundation headers.
- Genuine SpiderMonkey 1.8.5 generated cross-target `jsapi.h` and `js-config.h` from previous isolated native build scratch; not a guarantee JS engine runtime links or runs.
- GFOpenRTG `library/modules/sdl3` main at `6e3ae8e3abb98de4af01c577ed2663866d2fedc1`: ran its **actual** `build.sh <scratch-output> stub` with GCC16. It SHA-256 verified the real SDL 3.4.14 upstream tarball `30d4aa2b3037718142b32dffd4e72f917ebb6cc5227150e7bb9c45efb2153aeb`, applied pinned SDL3_68k and GFOpenRTG patches and produced canonical `include/SDL3/SDL_video.h` plus `libSDL3.a` stub; build RC0. **Did not build the full SDL3.module**, so no SDL3 runtime claim.
- GFOpenRTG's actual `library/modules/openal/include/AL/al.h`, instead of disabling audio or inventing OpenAL declarations. **Did not build or link OpenAL.module**.
- Existing GCC16 stove's `GL/gl.h`/`GL/glext.h`, with `GL_GLEXT_PROTOTYPES` to expose canonical exported OpenGL entry-point prototypes. These are not proof that all GL functionality will work or be linked.

## Evidence and remaining work

The `Universe-gfos-68040.o` object has a real unresolved `_OOGFOSGameFrameUpdate` reference, exactly as required for later linking to `OOGFOSGameFrame.o`. Full Oolite application/Universe linking has **not succeeded**: an exploratory `m68k-amigaos-ld -r` partial relocation of the class and the frame module returned `final link failed: bad value`. We do not describe that as successful linkage; diagnose linker/relocations in the following gate. This build also leaves many original OpenGL, Objective-C, scripting and game-engine dependencies for final linkage.

The previously qualified standalone frame-hook binary was re-linked with its corrected ObjC-safe header and passed again on actual AmigaOS 3.2.3 AC090 guest, RC0 (see `tests/evidence/oolite-universe-frame-objc-abi-guest.log`); this is **not** running the whole Universe object.

Next: compile/link the remaining real game modules and startup, resolve GL/SDL3/OpenAL linking against GFOpenGPU modules and native Foundation, close stalled JS sender lifetime semantics, then run actual Oolite title/gameplay. A successfully compiled Universe object is a milestone, **not a playable game**.
