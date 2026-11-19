# Oolite Vector3D on a matched javascript.library v2 engine

Date: 19 November 2026. Target: AmigaOS 3.2.3 AC090 68040, isolated
AmiCradle Team 1 Oolite JSAPI lab.

## Implementation

The OOGFOSJSVectorNativePrototypeAvailable function registers the actual
OoliteVector3D native class and tests its real JS_ClassPrototype callback.
The probe is strict: unlike the existing constructor compatibility fallback,
it refuses to accept a JavaScript-owned probe prototype as evidence of v2.

The native executable tests:
- new Vector3D(3,4,0) with real prototype identity and instanceof
- Vector3D([1,2,3]) with original JS Array conversion
- native field access, magnitude, add and interpolation
- constructor identity on the exact native class prototype

Run with make js-vector-v2-engine and the canonical v2 GFOpenSDK checkout.
Compiled successfully for 68040 with GCC 16.2. Binary SHA-256:
b19f01f574b2d81562ac7dfda998ce09d6a4edcc147959396a047511de0e52d1.

## Guest observation, not a pass

The existing isolated guest has v2 loader 3672 bytes, SHA-256
488c01fdb95facfd44af009ba85ae890e65bc9ffb2a9049a5dd0e2bcb2b1c7ee,
but the old 20,289,808-byte JavaScriptEngine, SHA-256
389091de698479a37fffbbddd7cf2dc86fbe91d388d1eb49af6c0cbcbbb0c362.

The guest test explicitly returned REQUIRES_MATCHED_V2_ENGINE / RC 77,
as expected. It did not pass a native v2 callback test. Exact ACAgent output:
tests/evidence/oolite-vector-v2-old-engine-skip-20261119.log.

## Required release action

Build JavaScriptEngine using the v2 jsl_core.cpp callback table from the
standalone GFOpenJavaScript source with the PINNED JavaScriptCore build,
record source and binary hashes, deploy library and engine together in an
isolated lab, rerun this exact test and require RC 0. Only then qualify
Oolite Vector3D as running on the new engine.

The old original Oolite object corpus remains 242 files / 53 still
SpiderMonkey-dependent / 101 unique unresolved JS symbols. This test does
not reduce that count; OOJavaScriptEngine.m, OOJSScript.m and the remaining
original host binding sources must still be migrated, rebuilt and relinked.
