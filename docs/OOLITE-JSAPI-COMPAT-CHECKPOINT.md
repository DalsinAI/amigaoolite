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

## Shared-ABI adapter expansion — later 19 November

The dedicated `AmiCradle Team 1 - Oolite JSAPI RC1` lab now qualifies three more
native 68040 test gates using the **canonical** installed `javascript.library`:

- `OOGFOSJSProperties.{h,c}`: property lookup/update, indexed array access
  and `JS_Call` with the correct receiver. Original-object style smoke script
  evaluated and returned 26, **PASS RC 0**.
- `OOGFOSJSReset`: transactional new `JSTask` creation followed by teardown
  of the old task; a global set before reset was absent afterwards, **PASS RC 0**.
  On task replacement, old JSValue handles must be discarded and host classes
  re-registered.
- `oolite_js_host_reset_gate.c`: an Oolite-style host class was released by
  old task teardown and re-registered; the release callback fired twice after
  reset and final shutdown, **PASS RC 0**.
- `OOGFOSJSValueToInt32`: implement ECMAScript truncation/modulo-2^32 and
  signed wrap via `JS_ToDouble`, not `JS_ToLong`; ten numeric boundary cases,
  **PASS RC 0**.

Guest evidence:
[`oolite-js-properties-reset-host-guest-20261119.log`](../tests/evidence/oolite-js-properties-reset-host-guest-20261119.log),
[`oolite-jsapi-int32-guest-20261119.log`](../tests/evidence/oolite-jsapi-int32-guest-20261119.log).

These primitives narrow the conversion work but are **not** a claim of a
SpiderMonkey-compatible binary ABI or complete Oolite OOJS object graph.
The original `OOJavaScriptEngine.m` still owns a `JSRuntime`, `JSContext`,
`JSObject` and SpiderMonkey-specific error/root management; all 101
unresolved `JS_*` original link symbols remain a build gate.

## Exception semantics — later 19 November

`OOGFOSJSEvaluateChecked` now provides an error-message boundary over the
canonical `JS_Evaluate`/`JS_ErrorText` calls. A native AmigaOS 68040 test
threw a deliberate JavaScript Error and verified `JSERR_EXCEPTION` plus
nonempty diagnostics, then successfully evaluated another script (`6*7=42`).
**Guest PASS RC 0**. Evidence:
[`tests/evidence/oolite-jsapi-errors-guest-20261119.log`](../tests/evidence/oolite-jsapi-errors-guest-20261119.log).

This helper does **not** emulate the historical SpiderMonkey `JSErrorReport`
layout, debugger callbacks or original error monitor. They remain explicit
migration requirements alongside the 101 symbol inventory.

## Further semantics — later 19 November

- Extended `OOGFOSJSValueToInt32` native test to **13/13 PASS**, including
  NaN, positive and negative infinity; see the source gate and the later
  transcript recorded in the test history. Unlike `JS_ToLong`, this performs
  explicit signed wrapping modulo `2^32`.
- Added `OOGFOSJSRetainValue`/`OOGFOSJSReleaseValue` using canonical
  `JS_Dup`/`JS_Free`. An object remained usable after removing its global
  reference, then its owned handle was released. **Native guest PASS RC 0**:
  [`tests/evidence/oolite-jsapi-retained-value-guest-20261119.log`](../tests/evidence/oolite-jsapi-retained-value-guest-20261119.log).

**Compatibility warning:** these handles are task-local; this does not emulate
SpiderMonkey's address-rooting ABI. Original Oolite callers must be adapted
to explicit retain/release and discard old values on reset. The 242-unit
original-engine link remains the next major gate.

## Host class hierarchy qualification — 19 November

The canonical `JSHostClass` parent-child model has now passed a native
Oolite-style inheritance gate. `OoliteEntityProbe` supplies `energy=32`;
`OoliteShipProbe` derives from it and supplies `cargo=10`. The child
wrapper exposes both fields and evaluates to 42. **AC090 guest PASS RC 0.**
Evidence:
[`tests/evidence/oolite-jsapi-host-inherit-guest-20261119.log`](../tests/evidence/oolite-jsapi-host-inherit-guest-20261119.log).

Basic slot inheritance is proved; the full historical `JS_InitClass` contract,
prototype construction, SpiderMonkey resolution hooks and Oolite OOJS object
identity are **not** proved. These remain in the original 101-symbol gap.

## Original Oolite engine-facing compatibility — 19 November (later)

Progress is now on the **original object and script boundaries**, not merely
generic JavaScript arithmetic:

| Port boundary | Native AC090 outcome | Evidence |
| --- | --- | --- |
| OOJavaScriptEngine reset preparation, commit and rollback | PASS RC 0; failed setup retains old script state; notification and finalizer order checked | [prepared reset](../tests/evidence/oolite-js-prepared-reset-guest-20261119.log) |
| OOJSScript original UTF-16 source to UTF-8 | PASS RC 0; BMP/supplementary Unicode and invalid surrogate cases tested | [UTF-16 source](../tests/evidence/oolite-jsapi-utf16-guest-20261119.log) |
| Unmodified Oolite default ship JavaScript loaded from UTF-16 and dispatched to ObjC fixture | PASS RC 0 | [single original script](../tests/evidence/oolite-original-utf16-objc-guest-20261119.log) |
| Two original default ship scripts with distinct ship identity on one JSTask | PASS RC 0, eight game-task RPCs | [two-script isolation](../tests/evidence/oolite-two-original-utf16-objc-guest-20261119.log) |
| Original OOJSEntity energy and maxEnergy | PASS RC 0; IEEE754 float via Exec and original clamping/positive-max rules | [energy and type flags](../tests/evidence/oolite-entity-flags-energy-guest-20261119.log) |
| Original OOJSEntity isValid/isShip/isStation/isPlayer/isDock/isInSpace | PASS RC 0; true JS Booleans and STATUS_DEAD transition | [energy and type flags](../tests/evidence/oolite-entity-flags-energy-guest-20261119.log) |
| Original ship scripts relinked with expanded Entity property host class | PASS RC 0, both one- and two-ship tests | [regression](../tests/evidence/oolite-entity-flags-energy-guest-20261119.log) |
| Synchronous game-task script invocation with Exec RPC pump | PASS RC 0; exception sends DONE and next script succeeds | [sync invocation](../tests/evidence/oolite-jsapi-sync-invoke-guest-20261119.log) |

Reusable interfaces:
`OOGFOSJSResetPrepared` (`OOJavaScriptEngine` reset),
`OOGFOSScriptObjectLoadUTF16` (`OOJSScript` source ingestion),
`OOGFOSScriptInvoke` (synchronous game-to-script task calls),
`OOGFOSScriptInvokeEvent` (synchronous per-ship event routing),
and expanded `OOGFOSShipClass` (native OOJSEntity scalar/type properties).

**Resolved deadlock:** the original `OOGFOSScriptInvokeEvent`
qualification initially hung because script initialization invoked game
callbacks while the game task was waiting on an unpumped `JS_Post`.
Initialization and event injection now use `OOGFOSScriptInvoke`; original
script events, one-shot deletion, exception recovery, all eight completion
messages and four real game-task actions **passed natively RC 0**.
See [original event guest evidence](../tests/evidence/oolite-jsapi-event-invoke-guest-20261119.log).

Additional source-level original Oolite passes (all actual AC090
guest runs, not host-only probes):

| Boundary | Result | Evidence |
| --- | --- | --- |
| `OOJavaScriptEngine -callJSFunction:`: arbitrary returned object, receiver/args, exception and next-call recovery | PASS RC 0 | [function calls](../tests/evidence/oolite-jsapi-call-function-guest-20261119.log) |
| Full *unmodified* `oolite-global-prefix.js` extending actual native `Ship.prototype.spawnOne` | PASS RC 0; other classes are explicit fixtures | [global prefix](../tests/evidence/oolite-original-global-prefix-guest-20261119.log) |
| OOJSScript `Script.prototype` inheritance, actual global lookup deletion, externally retained old script identity | PASS RC 0 | [script lifecycle](../tests/evidence/oolite-js-script-lifecycle-result-guest-20261119.log) |
| OOJSScript arbitrary owned result from `-callMethod:result:`, missing method, exception recovery across game-task pump | PASS RC 0, seven DONE | [script results](../tests/evidence/oolite-js-script-lifecycle-result-guest-20261119.log) |
| Original OOJSEntity read-only GLfloat collisionRadius/distanceTravelled/mass/spawnTime | PASS RC 0, strict write rejection | [read-only scalars](../tests/evidence/oolite-js-entity-readonly-scalars-guest-20261119.log) |

The production startup ordering issue found in the full global prefix
qualification is being addressed with an **append-only,
version-gated** `JS_ClassPrototype` vector in
[GFOpenBrowser PR #230](https://github.com/DalsinAI/gfopenbrowser/pull/230)
and [GFOpenSDK PR #73](https://github.com/DalsinAI/gfopensdk/pull/73).
`OOGFOSJSClassBootstrap` will use that to register `Ship` before any
ShipEntity exists. The patched `javascript.library` v2 cross-builds,
but the *existing v1 JavaScriptEngine* returned
`REQUIRES_V2_ENGINE` (expected native skip 77); see the
[v2 fallback evidence](../tests/evidence/oolite-javascript-library-v2-old-engine-guest-20261119.log).
The matching v2 JavaScriptEngine and full native class-boot test remain
unqualified. Do not merge the runtime ABI or advertise this as done.

**Release limit:** the full original `OOJavaScriptEngine.m` and `OOJS*`
graph still needs a source-port away from SpiderMonkey, and the original
242-unit game has not passed a full GFOS-only link, startup, cockpit or
flight qualification. These isolated native passes must not be described
as playable Oolite.


## Semantic caveats and acceptance gates

1. `JS_ToLong` is **not** a substitute for SpiderMonkey `JS_ValueToInt32`. The separate `OOGFOSJSValueToInt32` helper now has 13 native conversion cases, including NaN and infinities. Do not alias `JS_ToLong` to `JS_ValueToInt32`.
2. `JS_ToBool` has a Boolean result, not a `JSERR_*` failure code. The adapter returns success separately and exposes the converted Boolean through an out-parameter.
3. A JSValue is scoped to its owning JSTask; callback-provided borrowed values must not be `JS_Free`d. Do not expose game-owned pointers to the script task.
4. Next: complete original `OOJavaScriptEngine.m` and every `OOJS*` host-class source port (not merely adapters), bootstrap native classes without dummy instances using paired v2 JavaScriptEngine, link all 242 original translation units **without** `libjs_static.a`, then qualify title and gameplay.
5. Any future compatibility facade must define explicitly which SpiderMonkey API functions are exact, adapted or unsupported. Class hooks, pending exceptions, GC roots and synchronous callbacks need independent parity tests before the original game relies on them.

No game, original upstream code or third-party assets are copied into this record.
