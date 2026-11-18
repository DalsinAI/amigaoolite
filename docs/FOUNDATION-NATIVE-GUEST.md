# AmigaOolite — actual 68040 guest engine and Foundation proof

18 November 2026 UTC, tested in a **dedicated native AmigaChrome AC090 68040/FPU guest**, Kickstart 47.2, Workbench 47.100. Source pinned from OoliteProject/oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`. GNUstep core from GFOpenGNUstep v0.1.1-rc1 with native GCC 16.2, libobjc, GFOS Exec message-port classes and libffi FPU-correct archive.

## What actually runs

1. **Original Oolite galaxy RNG** (`src/Core/legacy_random.c`) linked as native 68040 executable and executed in actual guest:

```
OOLITE_RNG_GUEST_BEGIN
RANROT 1a272b34 5c1c39f3 46f7b5db
CHECKSUM 774 20090
OOLITE_RNG_GUEST_RC 0
OOLITE_RNG_GUEST_END
```

Those values **exactly match the Linux reference** (`make host-check`). Raw guest log `tests/evidence/oolite-rng-68040-guest.log`.

2. **Original Oolite Objective-C Foundation module** (`src/Core/OOIsNumberLiteral.m`) compiled **unmodified** with upstream `OOFoundation.h`, `OOLogging.h`, `OOFunctionAttributes.h` and `src/SDL/Comparison.h`, against the real GNUstep Foundation 1.31.1 headers and native GFOS libraries. A separate test invokes Oolite's `OOIsNumberLiteral(NSString*,BOOL)` on 12 positive/negative cases. Native executable links (sha256 `70f267216e715ee334348c6eceeefe58a6dc2cffdf2dce7681047c155995be20`) and executed in actual guest:

```
OOLITE_FOUNDATION_GUEST_BEGIN
OOLITE_FOUNDATION_PASS cases=12 failed=0
OOLITE_FOUNDATION_GUEST_RC 0
OOLITE_FOUNDATION_GUEST_END
```

Raw guest log `tests/evidence/oolite-foundation-68040-guest.log`.

**The same native executable is reproduced by this repo's `make foundation-number` target**, using `UPSTREAM=/path/to/pinned/oolite`, `GFOS_BASE_DIR`, `GFOS_LIB_DIR`, `GFOS_FFI_DIR` and `PREFIX` set to the live GFOS toolchain. Repro build passed, without compiler warnings or source edits. The upstream source files are **not vendored** in this repository. The original Oolite source and asset licences remain in effect; see README and upstream LICENSE.

## What is not yet complete

These are **engine-component smoke tests, not Oolite gameplay**. The rest of the Objective-C game engine, SpiderMonkey/NSPR for OXP JS, current SDL3 front end and OpenGPU/OpenGL/GLU mapping, OpenAudio, save games/resource paths and real hardware performance all remain unimplemented or unqualified. GNUstep Foundation itself still needs Exec-signal NSRunLoop dispatch, multi-task IPC, concurrency and cancellation, fuller networking/Unicode/XML/TLS. No Oolite RC until menu/trade/combat/OXP and audio/rendering actually run.

## Next concrete engine integration

Compile the upstream Oolite Objective-C **Foundation-only** utility modules and their direct dependencies using `foundation-number` as the linked baseline. Qualify filesystem/resource paths next; then adapt OpenGPU's GL/SDL3 interface and implement real SpiderMonkey/NSPR guest scripting. Do not claim a playable game from passing RNG/number parsing.
