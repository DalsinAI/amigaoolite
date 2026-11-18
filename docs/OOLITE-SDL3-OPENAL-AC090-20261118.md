# Native GFOpenGPU SDL3 and OpenAL qualification for Oolite (18 Nov 2026)

This qualification advances the native Oolite runtime prerequisites, **not** a playable Oolite executable. The original pinned Oolite `Universe.m`, `MyOpenGLView.m`, `GameController.m` and `main.m` compile as real GCC 16.2 m68k 68040 native objects (`docs/OOLITE-FOUR-NATIVE-ENGINE-OBJECTS-20261118.md`), but the full app remains unlinked.

Canonical provider: `DalsinAI/gfopenrtg` at `6e3ae8e3abb98de4af01c577ed2663866d2fedc1`. Build in isolated daletop scratch using its own `library/modules/sdl3/build.sh` and `library/modules/openal/build.sh`, GCC 16.2 stove, AHI developer headers from the OS 3.2 stove. No libraries were installed into any shared GCC stove, and no non-Oolite guest was touched.

## SDL3

Fetched genuine SDL3 3.4.14 tarball with GFOpenRTG pin and validated SHA-256 `30d4aa2b3037718142b32dffd4e72f917ebb6cc5227150e7bb9c45efb2153aeb`. The build applied GFOpenRTG patches plus its pinned SDL3_68k backend. Canonical **`SDL3.module` built successfully**, 1,424,648 bytes, SHA-256 `db0573448cda95d7b88cda2730cc57e6c3deb5454e45c4f3f3b73fb22c37faef`, as well as `libSDL3.a`, public `include/SDL3/`, and native test binaries. GFOpenRTG's baserel checker accepted known entries; FPCR clash scan reported none. Some source signedness warnings remain in SDL3 dialog backend; no performance claim.

The dedicated Oolite AC090 68040 AmigaOS 3.2.3 guest initially had no SDL3.module. Staged the new module under `LIBS:GFOpenGPU/` in **this lab only** and executed the built `rendercompare.m` guest test: real SDL3 software renderer vs GFOpenGPU renderer, 18 scene comparisons, guest RC 0. Most scenes: zero pixels exceeding its per-channel 24 tolerance. `geometry, wrapped and clamped one way` had 544 differing pixels (1.11%, maximum absolute difference 192); `viewport and clip` had 111 (0.23%, maximum 143), both accepted by the original test as `same`. This is an SDL renderer test, not Oolite OpenGL rendering.

## OpenAL

Built canonical **`OpenAL.module`**, 59,652 bytes, SHA-256 `9c6ddd370ef126221a9401ef7b78c46aef74d38b1f68d86daacc76b73ea4040b`, plus `libopenal.a`, `include/AL/`, and native OpenAL tests; baserel and FPCR checks passed. Backed up the dedicated guest's existing OpenAL module (SHA-256 `22f80cace9b1bbb389ded0723d524fdcaa2b2de3626b1635181c24ae67c14514`), temporarily staged the new module, then ran canonical `alinfo.m` native guest binary. `ALInfo` reported ALC 1.1, available AHI Voices and Mix devices, `AL_VERSION "1.1 OpenAL.module 2"`, `AL_RENDERER "AHI Voices"`, connected 44,100 Hz device, expected extension answers, `ALInfo: all as expected`, **guest RC 0**. Saved exact guest stdout as `tests/evidence/oolite-gfopenal-guest.log`.

After the test the pre-existing OpenAL module was **restored to exactly its previous SHA-256**. SDL3.module is present only in this dedicated lab. No shared deployment was changed.

## Next link and integration work

The four original Oolite object files remain separate, and `m68k-amigaos-ld -r` between `Universe-gfos-68040.o` and the frame module previously failed `final link failed: bad value` (see existing object-build docs). The compile environment had real GFOpenGPU GL prototypes, but successful GL module symbol resolution, full Objective-C engine dependency closure, SpiderMonkey runtime linkage, and startup/resource tests remain unfinished. The vendored `stb_image_write.h:840` -Wstringop-overflow warning seen while compiling `MyOpenGLView.m` must be investigated. Neither this SDL3 test nor the ALInfo test demonstrates a running Oolite title or gameplay.
