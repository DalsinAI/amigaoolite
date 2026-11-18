# Full original Oolite OOOXZManager.m — GCC 16.2 native 68040 object qualification

18 November 2026. OoliteProject/oolite pinned at `21978c37b2f11c8664adf3e2062efc1d0c875979`, its real OOOXZManager is ~2,000 lines of upstream Objective-C, *not* the smaller GFOS archive helper. Actual Oolite SpiderMonkey ABI is supplied by the separate pinned OoliteProject/mozillajs-linux source at `bc12f7124166c199925f3cea37d2732d3704213d`, with GFOS patch 0010 and verified guest execution.

## Verified source/build result

With pinned Oolite GFOS patches 0001, 0004, 0006, 0007, 0008 and 0009 already applied, the 50-line `patches/0011-gfos-oxz-manager-bootstrap-object.patch` adapts **three original upstream files**:

- `src/Core/OOOXZManager.m`: the OXZ manager only sends `resetTypedString` and `stringToClipboard:` to the game view, so when `OOLITE_GFOS=1` it declares those Objective-C selectors without importing SDL3's `SDL_video.h`. Non-GFOS retains the original `MyOpenGLView.h` import. This does not implement OpenGPU or a replacement display; missing implementation is still a game boot dependency.
- `src/Core/OOSound.h`: permits an explicit `OOLITE_OPENAL=0` build, retaining original OpenAL default otherwise. Boot-only audio-disable builds must NOT be mistaken for an OpenAudio backend.
- `src/Core/Universe.h`: only declares the OOSound-specific Objective-C category when the OpenAL sound implementation is enabled; avoids declaring a category on a nonexistent class in the explicitly audio-disabled path.

Patch replay using pristine prior-stage `OOOXZManager.m` plus pristine upstream `OOSound.h` and `Universe.h` was verified byte-for-byte for all three target files. The original full `OOOXZManager.m` compiled **successfully** to an actual m68k Amiga object under GCC 16.2 with `-x objective-c -std=gnu11 -O2 -m68040 -m68881 -noixemul`, the true GFOpenGNUstep Foundation headers, source and generated JavaScript headers, and explicit `-DOOLITE_GFOS=1 -DOOLITE_OPENAL=0 -DOOLITE_ESPEAK=0`. The existing original package manager methods remain in the object. A dedicated reproducible `make oxz-manager-object` target is provided. Both original exploratory compile and Makefile build succeeded; their binary hashes differ due to compiler input/include-path differences and are not claimed byte-identical.

First successful object: 102 KiB, SHA-256 `0910790c29825e0d88936916881c9b774f1f42160665a2d1acdfb385116ba6ec`. Object format: `file format amiga`, 68k architecture. There were four warnings: deliberate audio-disable notice and three compiler warnings that full `MyOpenGLView` class declarations are missing (selectors are declared in a narrowly scoped GFOS category). No compilation errors. The object has 45 unresolved external symbols from the actual engine/Foundation, which is normal for an unlinked object, but means **complete manager linkage and runtime are not yet established**.

## Outstanding work (honest gates)

1. Link real `OOOXZManager` object against its upstream `ResourceManager`, `GuiDisplayGen`, `Universe`, GUI, native OpenGPU display and other real dependency objects, along with GFOpenGNUstep, the OXZ native extraction backend, SpiderMonkey and the relevant installed libraries. No fake SDL3/GLU/OpenAL stubs.
2. Replace the explicit audio-disabled bootstrap with the real OpenAudio sound source/stream implementation; inspect upstream `OOSound.h`, `OOOpenAL.h`, `PlayerEntitySound` and `Universe` call sites. A complete playable build must have sound or an explicitly supported silent mode.
3. Confirm OXZ manager `extractOXZ`, manifest collection, dependency resolution and installation UI using actual Oolite game initialization. The standalone extraction tests and native JavaScript tests have already passed separately; they are not a completed *combined* engine runtime.
4. Qualify SpiderMonkey allocations/GC under more intensive workloads. Lightweight object, array, string, arithmetic and forced-GC test passed; two heavier JS cases stalled and their logs were archived in the dedicated AC090 lab diagnostics rather than falsely reported as passes.
5. The actual screen/game startup is not yet running. Do not publish a playable release claim before first window, menu, world startup, input, save/load and runtime logs are proven.

All source/evidence is in DalsinAI/amigaoolite, with upstream source and licensing retained in its pinned repositories. Team 4 multicore and other canonical SDKs were not modified.
