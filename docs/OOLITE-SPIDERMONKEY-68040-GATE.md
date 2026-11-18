# SpiderMonkey 1.8.5 native 68040 integration gate

18 November 2026. Oolite engine pinned at `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`; authoritative matching JavaScript source is `OoliteProject/mozillajs-linux@bc12f7124166c199925f3cea37d2732d3704213d` (Mozilla 2.0 / SpiderMonkey 1.8.5). Do not substitute a fake `jsapi.h` or silently switch to an incompatible modern SpiderMonkey ABI.

## Observed compilation boundary

After applying the existing GFOS GL header adapter (`0001`) and maths adapter (`0009`) to pinned Oolite, a GCC 16.2 `-m68040 -m68881 -noixemul -DOOLITE_GFOS=1` object compile of the original `src/Core/OOOXZManager.m` stops at:

```
src/Core/GuiDisplayGen.h:31:10: fatal error: jsapi.h: No such file or directory
```

The upstream `OOOXZManager.h` directly includes `GuiDisplayGen.h`, whose header graph drags in SpiderMonkey. Its implementation also includes GUI, Universe and scripting dependencies. The native OXZ backend's separate source/guest tests do **not** prove the entire upstream manager compiles or links.

Pinned JavaScript repository was checked out separately without altering Team 4 SDK or other labs. Standalone `tests/oolite_jsapi_header_probe.c` containing a genuine `#include <jsapi.h>` was compiled with native target GCC16 and pinned source `-I js/src`; first failure is:

```
js/src/jsapi.h:48:10: fatal error: js-config.h: No such file or directory
```

The checked-in `js/src/js-config.h.in` explicitly identifies `js-config.h` as a **configure-generated** target ABI header (threading, ctypes, stdint, tracer/JIT and word size). Hand-authoring it to make compilation pass would not prove a valid target ABI. On daletop the checked shell PATH did not expose Python 2.7; `pkg-config --modversion nspr` did not find NSPR and the pinned GCC16 target prefix search did not locate `prtypes.h` or `libnspr4.a`. The JavaScript repo's own README directs builds through `libjs.make`, a Python 2.7-era toolchain and `libnspr4-dev` on Linux, which is **not** a validated Amiga target recipe.

## Required implementation sequence

1. Establish a dedicated, reproducible m68k AmigaOS cross-build of the pinned SpiderMonkey 1.8.5 and matching NSPR, explicitly specifying big-endian target, 32-bit pointer model and threading capability. Keep host code generators distinct from guest objects; do not confuse Linux NSPR with Amiga NSPR.
2. Generate `js-config.h` and other configured public headers from target-tested configuration. First gate: compile the included `tests/oolite_jsapi_header_probe.c` using the produced SDK headers; then link against actual guest `libjs_static.a` and NSPR objects.
3. Run a minimal guest `JS_NewRuntime` / context / script-evaluation test with deterministic output and bounded execution. No claim of scripting until this passes.
4. Compile Oolite `GuiDisplayGen` and the complete original `OOOXZManager.m`, then integrate the already guest-qualified OXZ extraction backend.
5. Run a bounded actual Oolite launch test and preserve exit code, guest logs and screenshot evidence.

No production SpiderMonkey or NSPR build, guest JS runtime, or full Oolite startup has been completed in this checkpoint. The OXZ archive path, schema, real asset loading and native maths qualifications are separate and already published.
