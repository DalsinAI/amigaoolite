# Oolite shared JavaScript migration — implementation checkpoint

- Date: 2026-11-19
- Team: AmigaChrome Team 1 / SacredTrees
- Baseline: OoliteProject/oolite `21978c37b2f11c8664adf3e2062efc1d0c875979`
- Production engine: canonical GFOpenSDK `javascript.library` and GFOpenBrowser JavaScriptEngine
- Status: partial migration; **full original Oolite is not yet linked without SpiderMonkey and is not playable**

## Reusable JSAPI compatibility proposal

The architecture proposal is in [GFOS designs](https://github.com/DalsinAI/gfosdesign/blob/design/gfos-ideas-register-20261119/gfos/ideas/spidermonkey-compat/README.md). It proposes *source compatibility for a pinned subset* of SpiderMonkey 1.8.5 JSAPI, not plug-in binary compatibility. Oolite's game-owned Objective-C universe remains behind the per-game Exec IPC/token bridge.

The original Scripting sources still use `JS_NewNumberValue` (~137 call sites), `JS_ValueToNumber` (~99), `JS_ValueToInt32` (~60), `JS_ValueToBoolean` (~48), and `JS_InitClass` (~27). These counts are an inventory of lexical calls in the pinned upstream Scripting directory, not evidence that those functions have been converted.

## Implemented this checkpoint

- `OOGFOSJavaScript.c`: track open adapter owners so closing one task no longer clears the global canonical `JavaScriptBase` for another; repeated close is idempotent.
- `tests/oolite_jslib_two_owners_gate.c`: native regression proving first owner closes twice, second owner still evaluates, and library base clears after the final close.
- `OOGFOSJSValues.{h,c}`: explicit conversion seam using *opaque* canonical `JSValue` handles and `JSTask`; covers double, long, boolean and constructors with error-path checks. No SpiderMonkey `jsval` structs or static interpreter included.
- `tests/oolite_js_values_gate.c`, `make js-values`: cross-buildable AmigaOS 68040 value-conversion regression.

## Build qualification

The isolated GCC 16.2.0-open3 cross-compiler on daletop produced:

- `oolite-js-values-68040` SHA-256 `37e707d0af03eb8a8c8000d5540500b703f718f5069f08640075d7e854c1c90c`;
- `oolite-jslib-two-owners-68040` SHA-256 `d8e030d6ceb052d759c0b8e641a84105250b8221280d6e81c4dea031fb138ad7`.

**These are cross-build passes, not newly executed guest tests.** Prior guest tests for Oolite script objects and host classes are documented separately under `docs/OOLITE-JSLIB-GUEST-PASS.md`.

## Semantic caveats and acceptance gates

1. `JS_ToLong` is **not a proven substitute** for SpiderMonkey `JS_ValueToInt32` (`ToInt32` modulo semantics). The helper is named `OOGFOSJSValueToLong` until parity is established; do not alias it to the SpiderMonkey function.
2. `JS_ToBool` has a Boolean result, not a `JSERR_*` failure code. The adapter returns success separately and exposes the converted Boolean through an out-parameter.
3. A JSValue is scoped to its owning JSTask; callback-provided borrowed values must not be `JS_Free`d. Do not expose game-owned pointers to the script task.
4. Next: native guest execution of both new gates; property/prototype/array and GC-root adapter design; Oolite `OOJavaScriptEngine` lifecycle conversion; original OOJS host-class conversion; original 242-unit GFOS-only link **without** `libjs_static.a`; full title/gameplay tests.
5. Any future compatibility facade must define explicitly which SpiderMonkey API functions are exact, adapted or unsupported. Class hooks, pending exceptions, GC roots and synchronous callbacks need independent parity tests before the original game relies on them.

No game, original upstream code or third-party assets are copied into this record.
