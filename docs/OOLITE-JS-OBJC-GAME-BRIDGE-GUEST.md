# Oolite GFOS JavaScript -> Exec game task -> Objective-C selector bridge

18 November 2026, native AmigaOS 3.2.3 on dedicated AmiCradle AC090 68040 with GCC 16.2, canonical GFOS shared `javascript.library`/`JavaScriptEngine`, native GNU Objective-C runtime (`libobjc`), GFOpenGNUstep Foundation 1.31.1 and the game's task-owned Exec dispatcher. The test DOES NOT link or instantiate Oolite's full upstream `ShipEntity` or Universe.

## Build seam

Oolite's pinned `ShipEntity.h` directly includes `OOJSPropID.h`, which includes historical SpiderMonkey `jsapi.h`. Also, Amiga NDK `exec/types.h` defines 16-bit BOOL whereas GCC's GNU ObjC runtime defines 8-bit BOOL. Including full Exec headers in the same Objective-C translation unit therefore fails with conflicting typedefs. Rather than fabricate a JSAPI header or embed a second engine, the GFOS implementation uses **two translation units**: `src/Core/OOGFOSShipEntityBackend.c` compiles as native C with true GFOpenSDK/Exec headers, resolves a game-task-only generation token to an opaque object pointer, and calls two plain numeric C functions; `src/Core/OOGFOSShipEntityBackend.m` compiles with actual pinned upstream Oolite `OOTypes.h`, GNUstep Foundation, and a narrow selector projection matching the documented methods in upstream `ShipEntity.h` and logic in `OOJSShip.m`—without importing the SpiderMonkey-dependent header.

The projected methods are `[ship bounty]`, `[ship setBounty:withReason:]`, `[ship homeSystem]`, `[ship setHomeSystem:]`, `[ship maxEscortCount]`, `[ship setMaxEscortCount:]`, `[ship escortGroup]` and `[ship isPlayer]`. It clamps negative bounty/homeSystem as original Oolite scripting does; the bounty setter uses the original `kOOLegalStatusReasonByScript` enum from OOTypes; maxEscorts checks player-readonly, existing escort count, range and original MAX_ESCORTS=16. The existing GFOS scalar bridge uses signed 32-bit `LONG`; the 64-bit native Oolite `OOCreditsQuantity` getter is rejected if above INT32_MAX instead of silently truncated. Richer typed JS values and exact rules across more properties are subsequent work.

## Native guest proof

`tests/oolite_objc_ship_mock.m` is deliberately a **test fixture**, declaring selector-compatible `ShipEntity` and `OOShipGroup` classes with the original selector signatures; the real Oolite `ShipEntity.m` is NOT in the executable. `tests/oolite_js_shipentity_objc_gate.c` creates the mock ship on the game task, registers it in the generation-token registry, publishes the GFOS script host object, runs real JavaScript asynchronously, pumps six Exec property operations, and checks the live Objective-C object returns bounty 44, home system 42, maximum escorts 4, and script-specific legal status reason. It joins the JS job, unregisters the native object, closes the owner port and releases the Objective-C fixture.

```
OOLITE_JS_OBJC_BRIDGE bound=1 rpc=6 done=1 value=90 bounty=44 home=42 escorts=4 legal_reason=2 closed=1 status=PASS
OOLITE_JS_OBJC_REPRO_RC 0
```

GCC16/Makefile compiles and links the native 4.5 MB executable using GFOpenGNUstep archive, libffi, libobjc, pthreads and GFOS libraries; after an earlier direct-build guest pass, the **independently rebuilt Makefile executable** also passed in the same guest. `make js-objc-bridge` final test binary SHA-256 `ba8e6a8aa1f3cb857427949bcc87179837c809ff41fa6f9136ab88649cd42c5f`. The direct build differed in hash due to build commands/input paths, not claimed byte-identical. Actual guest output: `tests/evidence/oolite-js-objc-bridge-repro-guest.log`.

Remaining full-engine blockers: original `ShipEntity.m`/Universe source integration, complete original OOJSShip/world/player event API, graphics & SDL3/OpenGPU compat, sound/OpenAudio, native runtime performance, save/OXZ and OXP startup. Our projected Objective-C selector interface must be checked against pinned upstream declarations whenever upgraded; a passing test fixture is not proof original game entity implementation is runtime-ready. The Exec dispatcher still needs dead-owner bounded-wait handling before it becomes the default.
