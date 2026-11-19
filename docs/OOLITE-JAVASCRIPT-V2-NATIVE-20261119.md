# Oolite: real javascript.library v2 engine and class prototype qualification

Date: 19 November 2026. Lab: AmiCradle Team 1 - Oolite JSAPI RC1,
AmigaOS 3.2.3, AC090 68040/FPU. This is an isolated experimental
build/qualification, NOT a published GFOpenJavaScript 2.0.0-rc1.

## Engine construction and provenance

The original browser-era JavaScriptEngine used in the lab had the v1
engine table. It correctly reported REQUIRES_MATCHED_V2_ENGINE (RC 77)
when the strict Oolite Vector3D prototype gate ran.

A fresh real JavaScriptCore-based JavaScriptEngine was cross-compiled
using the standalone GFOpenJavaScript v2 sources and an existing
JSC+68040 build from team3-webkit-jit-20261022/build-Amiga:

- GFOpenJavaScript source baseline: c8085afbfcbf15db49a904dac50abf44bfcce370
- jsl_core.cpp SHA-256:
  98fddd1a0c2b946e31da6e159eb7552837bb4b451b38a5b1f05eafb2aa652199
- jsl_cache.cpp SHA-256:
  f5a16f3208e9426c58b7921117a136be25a100b05fa642e4f8e2e48a3e99973d
- JSC archive SHA-256:
  9f61f1c2ddb03e90dd5b8004ebe034957003e3ea9e6f23ef00b4c329b22e57bf
- Existing JSC/WebKit build fingerprint:
  42092cdfd6b93cc3ecca29538856b0ab138517ff8d75719e568c0f2520dd54ff
- GCC 16.2.0 (m68k-amigaos), 68040/68881, native AmigaOS loadseg format
- Engine stripped size: 37,918,816 bytes
- Engine SHA-256:
  d35f5a3846786549dc8b8a453eb813f71bd7bf32bf5564d89cfd3b4dcc69c923

The four engine/OS dispatcher objects and jsimage_host.c were compiled
against the pinned JSC build and linked with its existing JSC/WTF/PAL,
ICU and libnix dependencies. The separate browser OBCore/WebCore
archives were deliberately excluded. It is a real engine, not an
interpreter stub or JSAPI symbol shim.

## Native qualification with browser v2 loader

The initial test used the already-installed GFOpenBrowser
javascript.library 2.0 loader (3,672 bytes, SHA-256
488c01fdb95facfd44af009ba85ae890e65bc9ffb2a9049a5dd0e2bcb2b1c7ee).

**PASS RC 0** from native AC090:
- Oolite Vector3D real JS_ClassPrototype: v2=1, install=0,
  eval=0, answer=42, post=0, status=PASS
- Full Oolite Vector3D original arithmetic/array/constructor gate: RC 0
- Library native prototype/host class/task isolation probe: RC 0

The original OOGFOSJSClassBootstrap initially returned answer=0 despite
successful registration. Native diagnostics isolated the defect:
ship instanceof Ship was false; the real native prototype, constructor,
energy field and unconstructable-call error were correct.

Reason: a JavaScriptCore host callback object exposed as JS_NewFunction
is callable, but does not implement ordinary JavaScript instanceof
semantics simply because its prototype property is assigned.
The Oolite adapter now wraps that callback in a genuine JS constructor
function, connects it to JS_ClassPrototype and leaves the native callback
unconstructable. This does not fabricate a game-owned ShipEntity.

**PASS RC 0** after the adapter fix:
OOLITE_CLASS_BOOTSTRAP class=1 wrapper=1 rc=0 answer=42 status=PASS.

Raw logs:
- tests/evidence/oolite-vector-v2-new-engine-pass-20261119.log
- tests/evidence/oolite-class-bootstrap-v2-fixed-guest-20261119.log

## Remaining release gates

- Run the same suite with the standalone GFOpenJavaScript loader
  (3,696 bytes, SHA-256 d8920e912507b5b8e25a30e6b04599362fc915273fccb371915304d13e0e4d80)
  and preserve the complete guest output.
- Validate GC, reset, bytecode-cache invalidation, exception handling,
  and browser consumption with the exact release pair.
- Produce a reproducible standalone JSC/WebCore build and release
  provenance, not an unrepeatable lab-only link.
- The original pinned full Oolite corpus still has 242 original
  objects / 53 SpiderMonkey-referencing objects / 101 unresolved
  distinct JS_* APIs. This work establishes the new engine and adapter
  features; it does NOT claim the original full game link is clean or
  playable.

## Full standalone GFOpenJavaScript release-pair test (19 November)

The standalone CI javascript.library loader 3,696 bytes, SHA-256
d8920e912507b5b8e25a30e6b04599362fc915273fccb371915304d13e0e4d80,
was deployed to the same isolated AC090 lab with the new 37,918,816-byte
JavaScriptEngine SHA-256 d35f5a3846786549dc8b8a453eb813f71bd7bf32bf5564d89cfd3b4dcc69c923.

**ALL native tests PASS, RC 0:**
- strict Vector3D native prototype and constructor
- full Vector3D arithmetic, arrays and constructor regression
- corrected Oolite native Ship class bootstrap and instanceof identity
- native class field, identity, wrapping and task isolation
- standalone JS shell CLI itself

Verbatim guest log:
tests/evidence/oolite-gfopenjavascript-standalone-v2-pair-guest-20261119.log.
The first CLI EVAL invocation printed 67 rather than intended 42 due to
quoting; it nevertheless exited RC 0. A separate exact JS shell FILE
test printed 42 from print(6 * 7) with guest RC 0. Log:
tests/evidence/oolite-gfopenjavascript-standalone-v2-cli-42-guest-20261119.log.

Independent repeat cross-build from identical pinned JSC archive and canonical
GFOpenJavaScript source produced a byte-identical stripped JavaScriptEngine.
Reproducible build entry point:
DalsinAI/gfopenjavascript/scripts/build-javascript-engine-webcore-ninja.py.

The 101 original SpiderMonkey references remain unmodified. A source port
of the full original OOJavaScriptEngine and OOJS graph is still necessary
to remove them, with its own original-object rebuild and whole-game link.

## Further native v2 guest gates

Eight additional Oolite scripting gates passed on the exact standalone
v2 loader/engine pair: exception recovery, 13 signed Int32 boundary
cases, property access, host class reset/finalization, script prototype,
owned script result, full original Oolite global prefix and actual
Oolite world scripts. All RC 0. Raw transcript:
tests/evidence/oolite-gfopenjavascript-v2-extended-native-guest-20261119.log.

The original Vector3D/Quaternion native bindings and persistent Oolite
JavaScriptCore cache are separately qualified and documented in:
docs/OOLITE-QUATERNION-VECTOR3D-GUEST-20261119.md and
docs/OOLITE-JSC-SCRIPT-CACHE-GUEST-20261119.md.

The original 101 SpiderMonkey references are still a source migration
backlog. These new host bindings and native regressions establish
functional pieces needed for that port but do not replace the entire
original OOJS source graph.
