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


## Actual source migration checkpoint: 101 -> 100 (19 November)

One *original* Oolite translation unit has now been rebuilt without
SpiderMonkey: **Core_OORegExpMatcher_m.o**. The class's public Objective-C
selectors, original ivar layout, the NSString category and the legacy
0x01/0x04 regexp flag values are retained. Its RegExp implementation calls
the canonical JS library/JSC engine through a dedicated C bridge, not a
fake JSAPI symbol stub. The Objective-C source replacement and preserved
header are in src/Core/original-replacements; the portable bridge lives
in src/Core/OOGFOSRegExp.c.

The 68040 GCC16.2 cross-build produced an actual replacement Objective-C
object with **zero** unresolved SpiderMonkey JS_* symbols. Replaced only
that unit in a copied 242-object original corpus and reran the same
cross-nm inventory:

- **Before:** 242 original objects; 53 JSAPI-dependent; 101 unique JS_*
- **After:** 242 original positions, one real source-rebuilt; 52
  JSAPI-dependent; **100 unique JS_***.
- Removed genuine historical symbol: JS_NewUCRegExpObjectNoStatics.
- The GFOS OOGFOSRegExp.o companion uses canonical javascript.library
  vectors; it is not a binary SpiderMonkey ABI compatibility shim.

Repeat with make jsapi-regex-rebuilt-audit and the pinned original object
corpus; audit files are recorded at:
tests/evidence/oolite-rebuilt-original-object-jsapi-100-20261119.json
docs/OOLITE-JSAPI-LINK-AUDIT-100-20261119.md.

Native AC090 AmigaOS 3.2.3 runtime test of the *same C regex bridge*
passed all 10 cases: exact/case-fold/multiline matches, escaping, UTF-8,
invalid syntax/error recovery and unsupported flag rejection; guest RC 0.
Transcript:
tests/evidence/oolite-original-regex-gfos-guest-20261119.log.

**Scope warning:** the original Objective-C matcher object is compile
qualified; its selectors have not yet been invoked by a running Oolite
game. There is no complete game relink or evidence that the other 100
legacy symbols are gone. This is the first measured decrement, not a
claim of a playable Oolite build.

## Objective-C caller integration verified (19 November)

The actual original OORegExpMatcher Objective-C class and NSString
category are now linked against the real GFOS GNUstep Foundation
(with the JSC regexp C bridge), not merely cross-compiled independently.
The native AmigaOS 3.2.3 AC090 guest executed their public selectors:
exact match, case-sensitive mismatch, ignore-case, multiline, NSString
category dispatch, bad-regex rejection and recovery.

OOLITE_ORIGINAL_REGEX_OBJC checks=7 status=PASS, return code 0.
Actual guest transcript:
tests/evidence/oolite-original-regex-objc-guest-20261119.log.
Executable SHA-256:
fb525ea15b6eb7b838f5e720fe601992201fc6940d011076eadf996a8dfcbe45.

This establishes the original class ABI and Foundation/JSC integration.
It does not claim a full original Oolite executable link, or change
the measured 100 remaining SpiderMonkey APIs / 52 dependent objects.
