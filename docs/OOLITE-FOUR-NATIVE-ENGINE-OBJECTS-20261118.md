# Four original Oolite application modules cross-compiled to native AC090 68040 objects

18 November 2026, 14:55–15:00 UTC. Source OoliteProject/oolite pinned `21978c37b2f11c8664adf3e2062efc1d0c875979`. This is **real original source compilation** with GCC 16.2 -O2 -m68040 -m68881 -noixemul; not a stubbed application, not complete linking, not gameplay.

The `make oolite-core-four-objects` target compiles:

| Original Oolite source | Actual native object built | Purpose |
| --- | --- | --- |
| `src/Core/Universe.m` with opt-in GFOS frame-pump patch | `Universe-gfos-68040.o` 536 KiB | World updates and original object graph |
| `src/SDL/MyOpenGLView.m` | `MyOpenGLView-gfos-68040.o` 114 KiB | SDL3 window/input, graphics context and display glue |
| `src/Core/GameController.m` | `GameController-gfos-68040.o` 40 KiB | Startup/game session controller |
| `src/SDL/main.m` | `oolite-main-gfos-68040.o` 6.2 KiB | Original SDL application entry point |

**All four compile successfully, no fatal diagnostics.** The view reports an existing GCC 16 `-Wstringop-overflow` warning in Oolite's vendored `stb_image_write.h:840`, expanded at `:936`; it is not dismissed or assumed harmless. Retain and investigate before release.

Exact SHA256 of those four object files in the order above:

```
09a3c101892ee54b012a3f368b3f53b87cb75afb16c8229753f4bf71c3950e2c
345f3593e632bf37d7e1b9a9c909807ba4898083f474e2b3cc54cb17b24c377f
89cf16d3549e0cad9f83a6173034c175abae2ffc75cc3c77f9ef33636560aa6c
854aafc86d9ed2d74113fa51a46a725a9740b0d026ca74c7c0fde40b5d52764a
```

The verified pinned SDL3 3.4.14 source and GFOpenRTG's `library/modules/sdl3/build.sh <scratch> stub` produced the canonical SDL3 header tree and linker stub. GFOpenRTG also supplies the real OpenAL headers under `library/modules/openal/include`. Real native GFOpenGNUstep and existing cross-target SpiderMonkey 1.8.5 generated headers are included. Oolite's version macros are defined from the pinned git tag `1.93.1-255` and explicit test builder/date values (overridable in Makefile), to replace the Meson-generated version macros for this isolated build; **they are qualification values, not an official Oolite release identity**.

Upstream pinned checkout had existing GFOS source patches already applied (e.g. OOOpenGLOnly.h). The new Makefile recipe creates a separate patched copy of Universe.m, refuses a nonmatching upstream HEAD, and does not make local Git commits. The original SDL3, OpenAL and Oolite code is not vendored in this repo.

**Next:** build/link the full SDL3.module and OpenAL module (the compile used genuine headers but only SDL3's stub was generated); resolve the full Oolite object dependency closure, startup resource paths, and remaining JavaScript worker lifetime issues. The exploratory partial `ld -r` of Universe and frame hook returned `bad value` and must be diagnosed. No executable, title screen or gameplay pass is claimed.
