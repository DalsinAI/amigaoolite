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

- `oolite-js-values-68040` SHA-256 `1f09bc2f49e314f751f8f204ddc676433d9316bbf63c5c908109d88ead9efb81`;
- `oolite-jslib-two-owners-68040` SHA-256 `d8e030d6ceb052d759c0b8e641a84105250b8221280d6e81c4dea031fb138ad7`.

**Both gates have now passed in the isolated named AC090 68040 AmigaOS guest with RC 0.** The two-owner gate returned 42 then 56 after first-owner double close and cleared the base after final shutdown. The values gate checked 42.5, signed -42, true, undefined false and zero falsiness. Exact output: [`tests/evidence/oolite-jsapi-twoowners-values-guest-20261119.log`](../tests/evidence/oolite-jsapi-twoowners-values-guest-20261119.log). The dedicated lab is `team1-oolite-jsapi-rc1-20261119` (friendly name `AmiCradle Team 1 - Oolite JSAPI RC1`). This evidence concerns standalone adapter gates, not original Oolite full-engine execution. Prior guest tests for ship host objects remain in `docs/OOLITE-JSLIB-GUEST-PASS.md`.

## Further native integration passes — 19 November

The isolated Oolite lab also ran the original upstream `oolite-default-ship-script.js` through the existing shared JavaScript task and game-task dispatch bridges:

- `oolite-script-objects-68040`: two independent script objects, four dispatched callbacks, legacy ship actions, clean close — **guest PASS, RC 0**.
- `oolite-original-objc-event-68040`: original upstream ship script through selector-compatible Objective-C test backend, legacy method actions, clean close — **guest PASS, RC 0**.

Verbatim output and hashes: [`tests/evidence/oolite-original-script-objc-guest-20261119.log`](../tests/evidence/oolite-original-script-objc-guest-20261119.log). The Objective-C fixture is **not** a verified original full `ShipEntity` / `Universe` integration.

A direct `m68k-amigaos-nm -u` audit of all **242 original compiled translation units** found **101 unique unresolved SpiderMonkey `JS_*` symbols**, listed in [`docs/OOLITE-JSAPI-UNDEFINED-SYMBOLS-20261119.md`](OOLITE-JSAPI-UNDEFINED-SYMBOLS-20261119.md). They include the runtime/context, GC root, host class, exception and regexp-object surfaces. This is the actual conversion/link gap; isolated adapter tests do not remove it.

## Semantic caveats and acceptance gates

1. `JS_ToLong` is **not a proven substitute** for SpiderMonkey `JS_ValueToInt32` (`ToInt32` modulo semantics). The helper is named `OOGFOSJSValueToLong` until parity is established; do not alias it to the SpiderMonkey function.
2. `JS_ToBool` has a Boolean result, not a `JSERR_*` failure code. The adapter returns success separately and exposes the converted Boolean through an out-parameter.
3. A JSValue is scoped to its owning JSTask; callback-provided borrowed values must not be `JS_Free`d. Do not expose game-owned pointers to the script task.
4. Next: property/prototype/array and GC-root adapter design; Oolite `OOJavaScriptEngine` lifecycle conversion; original OOJS host-class conversion; original 242-unit GFOS-only link **without** `libjs_static.a`; full title/gameplay tests.
5. Any future compatibility facade must define explicitly which SpiderMonkey API functions are exact, adapted or unsupported. Class hooks, pending exceptions, GC roots and synchronous callbacks need independent parity tests before the original game relies on them.

No game, original upstream code or third-party assets are copied into this record.
