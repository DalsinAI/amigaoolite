# AmigaOolite — GFOS / OpenUp native AmigaOS port

**Status (18 November 2026): real pinned Oolite algorithms, Foundation resource parsing, Team 4 multicore board jobs and native OXZ archive extraction have passed in an AmigaChrome AC090 68040 guest. The original complete OOOXZManager and full Oolite engine are NOT linked or playable. No RC.**

## Official source and pinned baseline

- Upstream: https://github.com/OoliteProject/oolite
- Pinned source commit: `21978c37b2f11c8664adf3e2062efc1d0c875979` (12 November 2026, OoliteProject master).
- Baseline is current upstream Objective-C/GNUstep/Meson source, rather than the old legacy Oolite SDL2 port.
- **Code licensing:** GPL version 2 or later. **Art/configuration licensing:** GPLv2+ or Creative Commons BY-NC-SA 3.0 as described in the original `LICENSE.md`; retain attribution and do not relicense bundled artwork as our own or misrepresent it as unrestricted commercial material.

## Native toolchain — validated

The isolated OpenAmigaGCC 16.2.0-open3 contains an Objective-C compiler and the GNU Objective-C runtime `libobjc.a`. A basic Objective-C class compiled to native AmigaOS object format. Oolite's actual upstream `src/Core/legacy_random.c` world/galaxy generation module cross-compiled successfully for `-m68040 -m68881 -noixemul` and linked as a native AmigaOS loadseg executable. A host reference run produced:

```
RANROT 1a272b34 5c1c39f3 46f7b5db
CHECKSUM 774 20090
```

`tests/oolite_rng_gate.c` and `Makefile` reproduce the test against upstream source: set `UPSTREAM=/path/to/oolite` at the pinned commit, `PREFIX=/path/to/isolated/GCC16` and run `make host-check` and `make`. **This original RNG has subsequently been executed in the AC090 68040 guest**, with parity evidence under `tests/evidence/`; the paragraph above describes the original source-compilation gate. No need to commit a copy of the upstream Oolite source here.

## Engineering blockers for full game

1. **GNUstep Foundation/base:** The original GCC16 prefix lacked GNUstep Foundation, but native GFOpenGNUstep 1.31.1 archives are now built and guest-qualified for resource operations. Full NSRunLoop, GUI and game-engine Foundation integration still require qualification.
2. **Mozilla SpiderMonkey JS:** Current Meson Linux build requires static `js_static`/`nspr`. Oolite's OXP JavaScript API, scripts and expansion packs depend on it. Need a verified 68k-capable runtime integration, not a stub.
3. **Graphics:** Current source expects SDL3, OpenGL/GLU and OpenAL. Design GFOpenGPU OpenGL and SDL compatibility integration; do not bundle parallel GL/SDL stacks. Validate 68040 memory/performance and consider 68060/PiStorm/OpenGPU acceleration as main performance target.
4. **Audio:** Use GFOpenAudio/OpenUp native audio; datatypes for decoding supported files where appropriate. Validate OpenAL API and mixer semantics.
5. **Resources, saves and OXPs:** Map GNUstep paths and host-filesystem semantics into GFOS harddisk folders; preserve directory cases, Amiga metadata, savegames and Oolite Expansion Packs.
6. **Guest validation:** first full application startup, splash/menu, trading, hyperspace, combat, saves, OXP load, joystick, audio and measured frame rate; then standalone Installer and GFOpenUp Part. No RC before runtime proof.

This repo is a port overlay and validation home until a full source branch is ready. Keep upstream pinned and push all changes/evidence to GitHub; avoid local git commits.

## First OpenGPU geometry port patch — compiled

`patches/0001-gfos-native-GL-geometry.patch` adds a narrowly guarded `OOLITE_GFOS` path to Oolite's `src/Core/OOOpenGLOnly.h`, using the native `GFOpenGPU` `<GL/gl.h>` and `<GL/glext.h>` for geometry-only source. The upstream `OOPlanetData.c` icosphere mesh arrays then **cross-compile successfully** as a real 68040 native AmigaOS object. Run `make planet-object` after applying the patch to the pinned upstream tree. This proves the mesh data compilation; it **does not** imply Oolite's GLU, shader, SDL3 or full graphics engine is ported. Missing target GLU headers are a known later step, not concealed by a fake header.

## 18 November — native guest engine and Foundation qualification

The original Oolite galaxy RNG module now **runs in an AmigaChrome AC090 68040 guest**, with exact Linux-vs-Amiga Ranrot values/checksum and exit code 0. More importantly, an **unmodified upstream Oolite Objective-C Foundation module** (`OOIsNumberLiteral.m`) now compiles and links against native GFOpenGNUstep 1.31.1, and **12 of 12 Objective-C number-literal cases PASS in the guest (exit 0)**. The new `make foundation-number` target reproduces the Oolite+Foundation linked binary. Details and guest logs: `docs/FOUNDATION-NATIVE-GUEST.md`, `tests/evidence/`. This does *not* imply the full Oolite engine is playable or all Foundation APIs are implemented.

## Current verified native guest checkpoint — 18 November 2026

- **Real Oolite asset databases:** 8/8 upstream configuration files decoded through mounted AmigaDOS and native GFOpenGNUstep, including 194 ship definitions, 2,050 planetary entries and 1,186 descriptions. The original Oolite property-list parser passed its native guest gate separately. See `docs/REAL-OOLITE-RESOURCES-GUEST.md` and `docs/RESOURCE-PARSER-GUEST.md`.
- **Actual Team 4 board work:** Native `OMC_RunRange` processed 8,388,608 original Oolite RNG iterations; CPU0, board-eligible and *strict board-only* output matched checksum `081dedd2`, with guest RC 0. Emulated board result only, not a PiStorm physical performance claim. See `docs/OOLITE-MULTICORE-GUEST.md`.
- **Original OOOXZManager port (in progress):** Native AmigaDOS path methods, original Oolite MiniZip/zlib reader, OXZ decompression + native Foundation manifest parsing, path safety, archive preflight, extraction and failure rollback all have isolated 68040 guest passes. Actual pinned upstream `Config/shipdata.plist` was also repackaged for a controlled OXZ extraction test: native guest extracted 128,966 bytes and parsed 194 ships, RC 0. Upstream assets are not vendored into this overlay. See `docs/OOLITE-OXZ-REAL-ASSETS-GUEST.md` and `docs/OOLITE-OXZ-EXTRACTION-GUEST.md`. **Do not enable unattended arbitrary OXZ installation:** full manager and malicious archive hardening remain incomplete.
- **Native Oolite maths:** Original quaternion and rotation-matrix code with guarded GFOS libm include passed native guest math checks; see `docs/OOLITE-MATH-68040-GUEST.md`.
- **Next full-engine blocker:** Oolite requires *SpiderMonkey 1.8.5* / static `libjs_static.a` with NSPR. Original upstream `OOOXZManager.m` compile currently reaches `GuiDisplayGen.h` needing genuine `jsapi.h`; no target-compatible 68040 SpiderMonkey/NSPR is qualified. The guest CPU ABI is verified as 32-bit big-endian. See `docs/OOLITE-SPIDERMONKEY-185-NATIVE-GATE.md` and `docs/OOLITE-JS-ABI-68040-GUEST.md`.

Useful Makefile targets against pinned upstream source and isolated GCC16: `rng-omc-range`, `real-assets`, `upstream-plist`, `oxz-preflight`, `oxz-extract`, `oxz-schema`, `oxz-real-assets`, `math-68040`. Apply relevant numbered upstream patches as documented before invoking targets. Evidence under `tests/evidence/` records actual guest results; successful isolated targets are **not** equivalent to a full Oolite executable, title screen, rendering or playability.
