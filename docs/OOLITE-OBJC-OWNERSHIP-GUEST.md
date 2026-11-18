# Oolite game-task native Objective-C ship lifetime ownership

18 November 2026. Native AmigaOS 3.2.3 AC090 68040 running GFOS `javascript.library` on its own Exec task and GFOpenGNUstep Foundation/ObjC on the Oolite game task. The class called `ShipEntity` in this test is a controlled selector-compatible Objective-C fixture, **not** the pinned original Oolite `ShipEntity.m`; real game engine and Universe still unlinked.

`src/Core/OOGFOSShipEntityBackend.c` now has the generation-token registry **retain** each registered Objective-C ship through the C ABI functions `OOGFOSShipObjCRetain`/`OOGFOSShipObjCRelease` implemented in `src/Core/OOGFOSShipEntityBackend.m` (with no conflicting Amiga NDK `BOOL` in the ObjC unit). A failed token insertion releases the temporary retain; successful unregister removes the token first and then releases exactly the registry's strong reference. An outside Universe/controller may retain independent references; it must unregister any departed ship and drain/close JS requests before freeing context.

`tests/oolite_js_shipentity_ownership_gate.c` deliberately drops the caller's original reference immediately after registration, confirms the mock's `dealloc` did **not** run yet, evaluates ship scripts through the shared JavaScriptEngine, performs six native Exec IPC property operations and checks Oolite's script bounty legal-status reason. The test stops and joins the script task, unregisters the token and asserts the mock's dealloc counter incremented **exactly once** when the registry released its final strong reference. This test actively detects use-after-free or leaking registry retain semantics on this controlled case.

```
OOLITE_JS_OBJC_OWNERSHIP bound=1 rpc=6 done=1 value=90 bounty=44 home=42 escorts=4 legal_reason=2 retained=1 freed=1 closed=1 status=PASS
OOLITE_OBJ_OWN_REG4_RC 0
```

Four `make`-rebuilt native GFOS/ObjC programs (positive, clamp/range, player/group/64-bit guards, and ownership) were run sequentially on the isolated AC090 guest, all exit code zero. Rebuilt `js-ownership` executable SHA-256 `dcbe06b21f80c501ce145df91d6ba4799ffdcc7c0728f4f0f66d6b02f4331929`. Full guest output `tests/evidence/oolite-objc-registry-ownership-guest.log`.

This is not full Oolite runtime entity ownership: `ShipEntity` is still the controlled mock and main Game/Universe does not register/unregister live entities yet. There are no dead-owner Exec timeouts or complete OOJSShip/world object bindings. The registry's retained pointer is only used on the original game task; script task sees an opaque token and host wrapper. A game-task-only close/quiescence protocol must be observed when tearing down a live Universe.
