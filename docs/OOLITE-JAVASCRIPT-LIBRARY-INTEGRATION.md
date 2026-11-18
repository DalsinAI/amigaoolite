# Oolite GFOS scripting integration: use the canonical javascript.library

18 November 2026. Oolite upstream pinned `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`.

## Canonical source and ABI ownership

User/architecture direction: **reuse GFOS `javascript.library` rather than embedding another SpiderMonkey, NSPR or JavaScriptCore engine in Oolite**. Authoritative source and implementation: `DalsinAI/gfopenbrowser@065f443e18fe4a2d7cba19e747a8a58804850ae2/src/jslib` (JavaScriptCore, 68k JIT and a separate `JavaScriptEngine` task). Canonical SDK: `DalsinAI/gfopensdk@4ac683894edf8c0e30e36d9318662fdde0ed39c9/Include/{libraries,proto,inline,clib}/javascript*`; source-of-truth vector layout `SFD/javascript_lib.sfd`. Do not edit, fork, vendor or regenerate those SDK headers in this port. Runtime dependencies: `LIBS:javascript.library` version 1, `LIBS:JavaScript/JavaScriptEngine`, and for `JSON.parse`, `LIBS:json.library` version 1.

The existing 30 vectors expose `JS_NewTask`, `JS_EndTask`, `JS_Evaluate`, `JS_New*`, `JS_To*`, `JS_GetProp`/`JS_SetProp`, `JS_Call`, `JS_RegisterClass`, `JS_Wrap`/`JS_Unwrap` and `JS_ErrorText`. This is a GFOS task/value/host-class interface, **not** SpiderMonkey 1.8.5's `JSRuntime`/`JSContext`/`jsval`/GC roots API. Pasting `jsapi.h` or defining C macros is not ABI compatibility. Oolite's game scripting objects must be adapted as GFOS host classes, and lifecycle, exception, ownership, thread/Exec task and callback behavior must be validated on the guest. Host-class callbacks run in the script task and must not use the calling application's task-local C library state.

## First native integration seam

`src/Core/OOGFOSJavaScript.c` and `.h` are a small wrapper around the **actual SDK LVO inline calls**: `OpenLibrary`, create task, evaluate expression, convert integer, release JS value, end task and close library. No JavaScript engine code is compiled in Oolite. `tests/oolite_jslib_gate.c` tests `6*7 == 42` and returns 77 with a distinct `UNAVAILABLE` message if the shared library or its engine cannot be started. It returns 0 only upon successful live evaluation. The wrapper currently supports one owner/task per Amiga program because `JavaScriptBase` is process-global; **do not** use it as a drop-in Oolite JSAPI, share it across different library bases or call multiple instances concurrently without extending its ownership model.

`make jslib-bridge GFOS_JS_SDK=/path/to/gfopensdk` cross-compiles/links a native AmigaOS 68040 executable with GCC 16.2 and the genuine canonical SDK. No static SpiderMonkey or WebKit linkage is involved. This target passed on daletop on 18 Nov 2026. The output format was confirmed as `AmigaOS loadseg()ble executable/binary`.

### Isolated guest qualification: blocked by deployment

The dedicated AmigaChrome AC090 68040 guest was running and its prior JS staging job completed, but it did **not** have `javascript.library` or `JavaScriptEngine` installed in the inspected DH0/DH1 folder-backed devices. The adapter did start in that guest and printed:

```
OOLITE_JSLIB_ADAPTER_BEGIN
OOLITE_JSLIB status=UNAVAILABLE missing=javascript.library_or_engine
SYS:ACAgent/oolite-jslib-68040 failed returncode 77
```

**NOT a guest evaluation pass**. The runner left `run.running` on that nonzero return; the test and state were archived in the dedicated guest `diagnostics/oolite-jslib-20261118T0438Z/`, and the isolated runtime was shut down to avoid a stale blocked lab. This does not negate the separate pinned SpiderMonkey interpreter guest test in `tests/evidence/oolite-mozjs-68040-guest.log`; that prior probe did **not** exercise GFOS `javascript.library` and should not be used as proof of the shared library runtime. There are no duplicate runtimes added in the new Oolite code.

### Next hard gates

1. Locate or build **the existing GFOpenBrowser javascript.library and JavaScriptEngine artifacts** using the owning team's published build/CI; install with the owning team’s version into an isolated guest, alongside required json.library. Do not silently modify Team 2's browser lab.
2. Rerun this exact `make jslib-bridge` executable; require `value=42 status=PASS`, RC 0 and bounded shutdown.
3. Map Oolite's real `OOJavaScriptEngine`, universe and ship APIs to canonical GFOS host classes, and qualify host object identity, property/method access, exceptions and GC lifetime; preserve game behavior without pretending SpiderMonkey ABI identity.
4. Build original Oolite scripting and `OOOXZManager` modules against that adapter and the existing OpenGPU/GFOS libraries, then boot the game, capture screenshots and verify input/rendering.

## Repository merge status

At audit time, `DalsinAI/amigaoolite` had one branch (`main`) and no open pull requests. Earlier verified code/evidence was already on main. This SDK adapter and its build/documentation are additional commits on main; there is no pending pull request to merge. Do not claim a full-game release or runnable Oolite from a compile-only JavaScript adapter.

## Host-class seam (follow-up)

`OOGFOSJSExposeHostClass` and `OOGFOSJSSetGlobalFunction` are now present in the actual native adapter. They call the canonical `JS_RegisterClass`, `JS_Wrap`, `JS_Global`, `JS_SetProp`, `JS_NewFunction` and `JS_Free` vectors. `tests/oolite_jslib_hostclass_gate.c` defines an Oolite-style ship probe with two integer fields and a script-callable `tonnage()` method, then evaluates `ship.cargo + ship.fuel + (ship.tonnage() - 42)` expecting 42. The host callback creates its result through `JS_NewLong` and does not call the Oolite process's libc from the script engine task. This verifies the **build contract**, not real game ship objects or callbacks executing in guest.

The `make jslib-hostclass` target has cross-compiled and linked **byte-for-byte identically** with the direct GCC16 build, using canonical GFOpenSDK headers. Native executable SHA-256 `b0ad59fa5b99eb9e889af1b0f7195fe5eb9fa254ef8b75db9c25b3c75c814cad`. No new SDK code or separate runtime has been introduced. The class table, callback and native object must remain valid while a JS wrapper is live. Full Oolite entity ownership, GC release callbacks, locking and task-affinity still need integration and guest testing.

## CI/release artifacts located and staged

The owning `DalsinAI/gfopenbrowser` `jslib.yml` run **44203191890** completed successfully for revision `7538c366c…`; its published `javascript-library-68040` artifact contains `javascript.library` and the Shell `JS` program. The owning browser **v0.9.0-rc3** release includes `Libs/JavaScript/JavaScriptEngine` (about 20 MB), `Libs/json.library` and compatible ICU data. These specific binaries were copied into **only** the isolated Oolite guest folder-backed `DH0/Libs`, with ICU at `DH0/Libs/JavaScript/Unicode/icudt78b.dat`. SHA-256s as staged:

- javascript.library: `a0a03528a7adc7e905e5cc32a4982c2d9108f52a12f040b0b964575a035d0921`
- JavaScriptEngine: `389091de698479a37fffbbddd7cf2dc86fbe91d388d1eb49af6c0cbcbbb0c362`
- json.library: `a2f1143e0c167a7cc8fa21f398f312daba4dc6c3752f16fc5e9ba0b940c310f2`
- ICU data: `74dab575ac0bf687cb976cf84d25e608cd9cf2a19ce076cb1ec03bd46d9f1af0`

An immediate retry failed to complete because the **AmigaChrome native emulator was delayed in kernel buffer/disk I/O**, with the guest Shell script queued/running but no new command evidence. The matching host/bridge were brought up on localhost ports 9141/9140, and the emulator process was observed in uninterruptible disk waits (`wait_on_buffer`, `balance_dirty_pages`, `folio_wait_bit_common`). This is a host-lab reliability observation, **not proof of a `javascript.library` execution failure or success**. Preserve the guest script and outputs before recovery, and do not mark either the expression or host-class test as guest-passed until it emits RC 0 from the booted system.
