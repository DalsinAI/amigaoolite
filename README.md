# AmigaOolite — GFOS / OpenUp native AmigaOS port

**Status (18 November 2026): upstream discovered, first *real upstream source* native 68040 compilation passed. Full Oolite engine not yet built or playable. No RC.**

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

`tests/oolite_rng_gate.c` and `Makefile` reproduce the test against upstream source: set `UPSTREAM=/path/to/oolite` at the pinned commit, `PREFIX=/path/to/isolated/GCC16` and run `make host-check` and `make`. **The generated native test has not yet been executed in a GFOS guest**; endian/parity is still a guest gate. No need to commit a copy of the upstream Oolite source here.

## Engineering blockers for full game

1. **GNUstep Foundation/base:** Current Oolite heavily depends on Foundation classes, and the inspected isolated GCC16 prefix has `libobjc.a` but no target GNUstep Foundation headers or `libgnustep-base.a`. Port/qualify the required GNUstep subset or an equivalent compatibility framework before main Objective-C engine compile.
2. **Mozilla SpiderMonkey JS:** Current Meson Linux build requires static `js_static`/`nspr`. Oolite's OXP JavaScript API, scripts and expansion packs depend on it. Need a verified 68k-capable runtime integration, not a stub.
3. **Graphics:** Current source expects SDL3, OpenGL/GLU and OpenAL. Design GFOpenGPU OpenGL and SDL compatibility integration; do not bundle parallel GL/SDL stacks. Validate 68040 memory/performance and consider 68060/PiStorm/OpenGPU acceleration as main performance target.
4. **Audio:** Use GFOpenAudio/OpenUp native audio; datatypes for decoding supported files where appropriate. Validate OpenAL API and mixer semantics.
5. **Resources, saves and OXPs:** Map GNUstep paths and host-filesystem semantics into GFOS harddisk folders; preserve directory cases, Amiga metadata, savegames and Oolite Expansion Packs.
6. **Guest validation:** first full application startup, splash/menu, trading, hyperspace, combat, saves, OXP load, joystick, audio and measured frame rate; then standalone Installer and GFOpenUp Part. No RC before runtime proof.

This repo is a port overlay and validation home until a full source branch is ready. Keep upstream pinned and push all changes/evidence to GitHub; avoid local git commits.

## First OpenGPU geometry port patch — compiled

`patches/0001-gfos-native-GL-geometry.patch` adds a narrowly guarded `OOLITE_GFOS` path to Oolite's `src/Core/OOOpenGLOnly.h`, using the native `GFOpenGPU` `<GL/gl.h>` and `<GL/glext.h>` for geometry-only source. The upstream `OOPlanetData.c` icosphere mesh arrays then **cross-compile successfully** as a real 68040 native AmigaOS object. Run `make planet-object` after applying the patch to the pinned upstream tree. This proves the mesh data compilation; it **does not** imply Oolite's GLU, shader, SDL3 or full graphics engine is ported. Missing target GLU headers are a known later step, not concealed by a fake header.
