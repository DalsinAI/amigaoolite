# Oolite native-game / JavaScript behaviour boundary

Date: 19 November 2026. Repo: DalsinAI/amigaoolite.

## Decision

Keep Oolite's game state, physics, entities, numeric mathematics and game
event routing native in C/Objective-C. Use the shared canonical
GFOpenJavaScript JavaScriptCore interpreter to run existing JavaScript and
OXP-authored behaviour, with host-class properties/methods as the boundary.

Original Oolite uses one SpiderMonkey runtime and its main JSContext.
World scripts intentionally share a script environment: OXPs may inspect
other worldScripts, and scripts depend on game-owned wrappers. Do not split
every OXP into separate JSTasks until cross-script semantics and lifecycle
ownership have been addressed.

## Tested integration

A real m68k GCC16.2 68040 native executable uses:

- A game-owned C Ship with native bounty=2. Script-facing host class exposes
  bounty as a field; the game retains ownership of the native data.
- One canonical javascript.library v2 JSTask, without a SpiderMonkey shim.
- Two independent script objects, registered as oxp.a and oxp-b in the
  original-style read-only worldScripts registry. Scripts can communicate.
- Native-owned Vector3D and Quaternion host classes. OXP A uses
  Quaternion.rotateZ and Vector3D.rotateBy; returns the rotated vector.
- OXP B calls OXP A's event through worldScripts, combines the rotated
  vector with the C ship bounty, and returns 42.
- The C game reads the actual native Vector3D host wrapper and verifies
  x=0, y=1, z=0. Both event handlers fire, and ship bounty is unchanged.
  JSValue handles are freed before JSTask destruction.

There is no redundant JavaScript game/physics engine.

## Reproducible native evidence

Build: make js-native-oxp-boundary with pinned GCC16.2 and GFOpenSDK v2.
AmiCradle AC090 / AmigaOS 3.2.3 executed the 68040 binary:

    OOLITE_NATIVE_OXP_BOUNDARY exposed=1 rc=0 vec=0 quat=0 registry=0 oxp=0,0 event=0,0 calls=1,1 native=(0.000,1.000,0.000) gameBounty=2 result=42 status=PASS
    OOLITE_NATIVE_OXP_BOUNDARY_RC 0

Transcript: tests/evidence/oolite-native-oxp-boundary-guest-20261119.log.

## Remaining work

This integration gate exercises OXP-equivalent behaviour, not an original
Oolite executable or unchanged packaged OXP. The source currently retains
100 distinct SpiderMonkey symbols across 52 original object files.
No claim is made that this test reduces those numbers or makes Oolite playable.

Next: migrate OOJavaScriptEngine and OOJSScript from SpiderMonkey lifecycle
and XDR storage to current JSTask/script-object/cached-source functions,
then bind actual Oolite Objective-C entity methods and events. Preserve
shared OXP event semantics. Cancellation needs a verified nonblocking,
per-task interrupt API; never forcibly terminate shared JavaScriptCore.
