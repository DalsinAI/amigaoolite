# Original OOJSScript-worldScripts integration — 19 November 2026

Source-level, canonical GFOpenJavaScript v2, no SpiderMonkey runtime.
The existing shared JSTask, original Oolite UTF-16 loader, per-script
this-object model and worldScripts registry were REUSED.

## Original Oolite behaviour now exercised natively

- OOGFOSScriptObjectCreate(task, scriptGlobal, NULL) creates a global/world
  script WITHOUT inventing a ShipEntity or exposing a fake ship property.
  Ship-backed scripts still require a valid registered native wrapper.
- OOGFOSScriptObjectSetProperty(..., readonly=1) uses JavaScriptCore's REAL
  Object.defineProperty with an owned JS descriptor; value handles are
  borrowed. Readonly, nonconfigurable and enumerable attributes verified.
- Mutable script properties can be overwritten by original OXP JavaScript.
- OOGFOSScriptObjectGetProperty returns an owned canonical JSValue.
- Original worldScripts registry references global script and ship-backed
  scripts on the SAME shared JSTask, with inter-script calls.

Native test on AmigaOS 3.2.3 AC090 68040, stable GFOpenJavaScript v2:
tests/evidence/oolite-worldscript-metadata-original-semantics-native-20261119.log.
The OXP A and B interscript Vector3D/Quaternion event test continues to
return 42; global-only world script event returns 42; manifest-style
readonly author is unchanged after attempted JS overwrite; mutable counter
changes from 7 to 8.

## Next release integration

The original Objective-C OOJSScript.m and OOJavaScriptEngine.m still contain
SpiderMonkey APIs and must call the above native functions from game-task
safe JS_Post/Exec orchestration, not direct JS task calls. This test alone
is NOT the original class replacement and NOT playable Oolite.
Full original-object audit remains 242/50/98 (objects/JS dependent/
unique APIs), unchanged by this isolated adapter semantic correction.
