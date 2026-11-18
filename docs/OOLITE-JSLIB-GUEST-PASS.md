# Canonical GFOS javascript.library: real Oolite native 68040 guest qualification

18 November 2026. Source baseline `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`.

**Use the existing GFOS `javascript.library`/JavaScriptCore implementation. Do not bundle SpiderMonkey, NSPR, or another JS engine in Oolite.** Canonical SDK `DalsinAI/gfopensdk@4ac683894edf8c0e30e36d9318662fdde0ed39c9`; implementation `DalsinAI/gfopenbrowser@065f443e18fe4a2d7cba19e747a8a58804850ae2`.

## Exact owner-built guest runtime components

- `javascript.library` version 1: successful `DalsinAI/gfopenbrowser` CI run 44203191890, artifact `javascript-library-68040`, SHA-256 `a0a03528a7adc7e905e5cc32a4982c2d9108f52a12f040b0b964575a035d0921`.
- `JavaScriptEngine`: owner’s GFOpenBrowser `v0.9.0-rc3` LHA, SHA-256 `389091de698479a37fffbbddd7cf2dc86fbe91d388d1eb49af6c0cbcbbb0c362`, installed `LIBS:JavaScript/JavaScriptEngine`.
- `json.library`: same RC3 release, SHA-256 `a2f1143e0c167a7cc8fa21f398f312daba4dc6c3752f16fc5e9ba0b940c310f2`, installed `LIBS:json.library`.
- **Critical dependency:** `unicode.library` v1.1 from same RC3 release, SHA-256 `6136e3a975b418f22b6a89d77d0a08d0190822cd3215327285e34df89e9ed797`, installed `LIBS:unicode.library`.
- ICU data `icudt78b.dat` SHA-256 `74dab575ac0bf687cb976cf84d25e608cd9cf2a19ce076cb1ec03bd46d9f1af0`, installed `LIBS:Unicode/icudt78b.dat`, which is where unicode.library actually searches. The previously assumed `LIBS:JavaScript/Unicode` is not sufficient. Only the dedicated Oolite AC090 test image was modified, not Team 2’s browser labs.

## Native guest results

`tests/oolite_jslib_install_probe.c` used AmigaDOS `OpenLibrary`/`Lock` and verified all required library and engine paths in the guest, exit code 0:

```
OOLITE_JSLIB_DEPLOY lib=1 engine_libs=1 engine_sys=1 json=1 status=PASS
OOLITE_JSLIB_PATHS_RC 0
```

Manually launching the owner’s engine registered the canonical `JAVASCRIPT.ENGINE` port. `tests/oolite_jslib_gate.c` linked with `src/Core/OOGFOSJavaScript.c` then evaluated `6*7` through shared JavaScriptCore and returned:

```
OOLITE_JSLIB rc=0 value=42 status=PASS
OOLITE_JSLIB_RUNTIME_RC 0
```

This was repeated after fresh startup (`tests/evidence/oolite-jslib-eval-guest.log`, `OOLITE_JSLIB_RUNTIME_REPRO_RC 0`). Real object wrapping through `tests/oolite_jslib_hostclass_gate.c` registered an Oolite-style C ship host class with fields and a native `tonnage()` callback and evaluated through the shared engine:

```
OOLITE_JSLIB_HOSTCLASS registered=1 rc=0 value=42 status=PASS
OOLITE_JSLIB_HOSTCLASS_RC 0
```

Actual log: `tests/evidence/oolite-jslib-hostclass-guest.log`.

**Cold engine launch passed:** After `Wait 75`, engine port probe saw `JAVASCRIPT.ENGINE found=0 status=NO_PORT`. Without manually launching the engine, the *next* Oolite JavaScript adapter call automatically started `JavaScriptEngine` and evaluated `6*7` with `rc=0 value=42 status=PASS`, `OOLITE_JSLIB_COLD_EVAL_RC 0`. Guest log: `tests/evidence/oolite-jslib-coldstart-guest.log`.

**Real upstream scripts passed:** `tests/oolite_real_js_guest_gate.c` loaded the *unaltered* pinned upstream `Resources/Scripts/oolite-default-player-script.js` (1,110 bytes) and `oolite-default-effect-script.js` (1,185 bytes) from guest `SYS:Oolite/Scripts`, ran both through the shared GFOS engine, validated each `this.name`, and exited 0. Neither script is vendored into this repo; stage from pinned upstream source. Test log `tests/evidence/oolite-real-js-guest.log`:

```
OOLITE_REAL_JS name=oolite-default-player-script bytes=1110 opened=1 rc=0 property=1 status=PASS
OOLITE_REAL_JS name=oolite-default-effect-script bytes=1185 opened=1 rc=0 property=1 status=PASS
OOLITE_REAL_JS_TOTAL passed=2 total=2 status=PASS
OOLITE_REAL_JS_RC 0
```

Tests were native GCC16 `-m68040 -m68881 -noixemul`, executed in the isolated AmigaChrome AC090 68040 guest, not Linux host simulations. Native `make jslib-bridge`, `jslib-hostclass`, `jslib-install`, and `real-js` targets build real AmigaOS binaries against canonical SDK. The real-JS cross-build was independently rebuilt byte-for-byte identical to its guest-tested executable (SHA-256 `964e262d38ae007bdad823245ec95883369c2683cbaea64366ef566f7dddfc6f`).

## Not yet complete

No entire Oolite engine launch or title screen yet. Oolite’s upstream SpiderMonkey-specific `JSRuntime`, `JSContext`, `JSClass`, GC root, exception and scripting machinery is **not** ABI-identical to GFOS `javascript.library`. The genuine GFOS host-class API can be used to port it incrementally, but callback lifetimes, multiple script owners, game entity wrappers, mission/world/ship APIs and task affinity need explicit qualification. Only tested script fixtures and the synthetic ship host-class have passed; no unsupported arbitrary scripts, full OXZ manager or gameplay claim.
