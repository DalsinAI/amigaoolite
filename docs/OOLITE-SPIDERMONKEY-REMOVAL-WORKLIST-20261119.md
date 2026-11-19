# Oolite / GFOS: remove embedded SpiderMonkey from the original game build

**19 November 2026.** Pinned upstream: OoliteProject/oolite
21978c37b2f11c8664adf3e2062efc1d0c875979. Source-level migration only;
do not replace the historical SpiderMonkey runtime with a fake binary JSAPI
or a table of 101 no-op linker symbols.

## Verified baseline

The original pinned 68040 object corpus has **242 compiled translation
units**, of which **53** reference **101 distinct unresolved SpiderMonkey
JS_* symbols**. Reproduced using native GCC 16.2 m68k-amigaos-nm -u,
with exact module-by-module output in the companion JSON and report:

- tests/evidence/jsapi-unresolved-by-object-20261119.json
- docs/OOLITE-JSAPI-LINK-AUDIT-20261119.md
- tools/jsapi_link_audit.py (rerunnable; accepts reductions, rejects new or unclassified API dependencies)

These are original *unconverted* object files. The count is not lower merely
because GFOS standalone test adapters have passed.

## Migration order, by actual link hot spots

| Original upstream unit | Distinct historical JS symbols | Conversion boundary |
|---|---:|---|
| OOJavaScriptEngine.m | 48 | Replace runtime/context/global creation, reset, GC/root/error and callbacks with a JSTask-based engine lifecycle. Preserve game-thread dispatch and exception handling. |
| OOJSScript.m | 28 | Reuse native UTF-16 source and JS_Evaluate, JS_Call and explicit JSValue ownership. Preserve call-method semantics, script identity and failure recovery. |
| OOJSMission.m | 20 | Port mission host properties and native game-task messages; preserve mission state and per-script lifetime. |
| OOJSVector.m | 19 | Replace SpiderMonkey-specific class implementation with existing OOGFOSJSVector host class, but add missing original methods (quaternion rotations, random-vector family, coordinate-system conversion) and original conversion interfaces. |
| OOJSQuaternion.m | 19 | Add native Quaternion host class and constructor/prototype/value semantics; do not stub rotation methods. |
| OOJSTimer.m | 19 | Task-safe scheduled callbacks and owned JS references; preserve cancellation, callback ordering and reset. |
| OOJSSystemInfo.m | 17 | Migrate host properties and functional callbacks to canonical task-bound APIs. |
| OOJSGlobal.m | 15 | Establish source-level canonical global class registration, properties and startup ordering. |
| OOJSPlayerShip.m | 14 | Bridge original ship lifecycle and native property dispatch; no direct script-task dereference of game objects. |
| OOJSSystem.m | 14 | System host bindings and validity; use the game-owner IPC and tokens. |

The remaining 43 dependent modules must be migrated too; the full report
names every original unit. This order is for engineering dependency and risk,
not a promise that ten files eliminate all 101 external names.

## What is reusable now

- GFOpenJavaScript owns the canonical JavaScriptCore/68k JIT runtime and
  versioned javascript.library; GFOpenSDK owns its public headers and vectors.
- OOGFOSJSValues handles explicit number/boolean/int32 conversions and
  JS_Dup/JS_Free retention without treating JSValue like SpiderMonkey jsval.
- OOGFOSJSClassBootstrap consumes JS_ClassPrototype (v2) to expose a true
  class prototype without fabricating a game-owned object.
- OOGFOSScriptObjects, OOGFOSScriptInvoke, prepared reset, native host
  hierarchy, ship properties/tokens and Exec IPC have bounded isolated
  native tests. Those tests do not constitute a fully linked original engine.
- OOGFOSJSVector implements substantial scalar/vector math with
  Vector3D constructor/prototype semantics; its known original API gaps
  are explicitly listed above.

## Semantics that a thin adapter cannot silently invent

1. **JSTask is not JSContext.** No one-to-one equivalent for a SpiderMonkey
   runtime/context/compartment API. Replace lifecycle at original source sites.
2. **JSValue is an owned opaque task-bound handle.** Replace address roots
   with explicit duplication/freeing and discard handles after task reset.
3. **Original game entities are game-task-owned.** JSHostClass callbacks run
   on the JavaScript task; use bounded Exec IPC and token generation checks.
4. **Class construction matters.** v2 JS_ClassPrototype exposes a real
   prototype, but constructor behavior, inheritance and instanceof need
   original source and native regression verification.
5. **Exceptions, JSAPI regexp objects and XDR are not equivalent** to
   JS_ErrorText, a regex library or JSC disk cache without explicit
   behavior/compatibility testing.
6. **No competing compiler/GC/cache.** Use JS_Evaluate and JSA_CacheDir;
   preserve OXP script behavior, not historical engine internals.

## Release gates

- Build and native-qualify a matched standalone GFOpenJavaScript v2 library
  AND JavaScriptEngine pair. The 68040 CI loader artifact alone is not enough.
- Convert original OOJavaScriptEngine and OOJSScript source into GFOS-native
  lifecycle/script execution, preserving upstream data types and behavior.
- Migrate remaining 53 original translation-unit dependencies. At each
  checkpoint rerun the exact object audit; report counts only on actual
  newly compiled GFOS objects, never by merely excluding objects.
- **Final source/link:** all original game translation units compiled, zero
  unresolved SpiderMonkey JS_* references, zero libjs_static.a/NSPR engine
  dependency, complete GFOS-only executable linked.
- **Final runtime:** AmigaOS 3.2.3 AC090 title/startup, OXP scripts,
  flight/AI, save/load, input/audio and crash-free shutdown.
- Only then declare an Oolite RC candidate.

Do not archive or delete the old experimental SpiderMonkey gate until its
comparative evidence has been retained. It must never become a production
link dependency.
