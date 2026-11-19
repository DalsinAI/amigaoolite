# Oolite Vector3D: original JS Array conversion via javascript.library

**Date:** 19 November 2026. **Target:** isolated
AmiCradle Team 1 - Oolite JSAPI RC1, AmigaOS 3.2.3 AC090 68040.

## Implemented

- OOGFOSJSVectorReadValue accepts canonical native Vector3D wrappers
  or genuine JavaScript arrays of exactly three finite numbers.
- Host vector methods now accept array arguments for original
  OOJSVector conversion parity: add, subtract, dot, cross,
  distance/squared distance, angle, triple product and static interpolation.
- The constructor accepts Vector3D([x,y,z]) alongside its original zero,
  triple numeric and native-vector forms.
- Uses JavaScriptCore's real Array.isArray through the library, indexed reads,
  JS_ToDouble and the owning JSTask; no SpiderMonkey jsval or guest entity access.
- Array-like plain objects, short arrays and non-finite coordinates are
  rejected. The test checks subsequent JS evaluation still works.
- Exact-wrapper-only OOGFOSJSVectorRead remains unchanged;
  OOGFOSJSVectorReadValue is the expanded conversion interface.
- **Not implemented:** Entity -> position, quaternion rotations,
  coordinate-system conversion and random-vector static methods.

## Qualification

Compiled and linked with OpenAmigaGCC 16.2 native m68k-amigaos GCC,
GFOpenSDK at 9433cc8d2b1b363b095f0af14e3c114f2aebb22a.
Build target: make js-vector. Binary SHA-256:
57e2a93da64c08554e40c04dcba9a47d31825bbf57e70c06417fbc2437c68b4b.

Installed guest runtime **was not the new v2 engine**:
- javascript.library 2.0, 3,672 bytes,
  SHA-256 488c01fdb95facfd44af009ba85ae890e65bc9ffb2a9049a5dd0e2bcb2b1c7ee
- Older JavaScriptEngine, 20,289,808 bytes,
  SHA-256 389091de698479a37fffbbddd7cf2dc86fbe91d388d1eb49af6c0cbcbbb0c362

The engine therefore uses the deliberately retained JavaScript-owned
prototype fallback. **This is not v2 JS_ClassPrototype qualification.**

Actual guest ACAgent output is preserved verbatim in
tests/evidence/oolite-vector-array-guest-20261119.log:

- valid=42, native coordinate conversion success, new constructor 42
- short array rejected (JSERR_EXCEPTION=1)
- array-like object rejected (JSERR_EXCEPTION=1)
- infinity rejected (JSERR_EXCEPTION=1)
- evaluation after errors recovered (42)
- guest test **PASS, RC 0**, ACAgent execution completed

This qualifies the native vector adapter, not original OOJSVector.m
or the complete game. The original object corpus still contains 101
unresolved SpiderMonkey symbols.
