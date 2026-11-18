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
