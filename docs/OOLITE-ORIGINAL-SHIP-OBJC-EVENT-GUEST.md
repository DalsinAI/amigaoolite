# Original pinned Oolite ship script -> GFOS Javascript -> game-owner ObjC action callback

18 November 2026, actual AmigaOS 3.2.3 guest on AmiCradle AC090 68040 with canonical GFOS `javascript.library` and GFOpenGNUstep Foundation. Original script loaded unmodified from the pinned `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979` file `Resources/Scripts/oolite-default-ship-script.js` (3,757 bytes), staged in the guest at `SYS:Oolite/Scripts/oolite-default-ship-script.js`. **Upstream script is not vendored** into this overlay.

`src/Core/OOGFOSShipEvents.{c,h}` extends the existing GFOS `OOGFOSShipClass` host-class parent with `__runLegacyScriptActions`. The script-task callback obtains the action-list length from actual JS input and sends `OOGFOS_DISPATCH_ACTION` through the tracked game-task Exec port. New optional game-owner action callback registration via `OOGFOSGameDispatchSetAction` occurs before any client messages. The owner resolves the opaque generation token, validates the action ID, and calls a native Objective-C action receiver. The C->ObjC boundary continues to use GFOpenGNUstep and the token registry's strong-reference ownership. No native game function is invoked on the JavaScript task.

`tests/oolite_js_original_ship_event_objc_gate.c` evaluates the real ship script asynchronously on `JS_Post`, with original `legacy_setupActions`, `legacy_launchActions` and `legacy_deathActions` event properties. The script itself invokes the new host method once during setup, again from `shipSpawned` and again from `shipDied`. A subsequent inherited `ship.bounty` property read also traverses the game task. The game-task mock Objective-C ship counts **three action callbacks covering four action-list elements** without interpreting them. The script's one-shot spawn handler is confirmed deleted. The test exercises the canonical GFOS host-class parent inheritance and real on-guest Objective-C method dispatch.

```
OOLITE_ORIGINAL_OBJC_EVENT bytes=3757 bound=1 methods=3 actions=4 rpc=4 completed=1 result=1 closed=1 status=PASS
OOLITE_QA14_RC 0
```

Reproducible `make js-original-objc-event` executable SHA-256 `6221720bf08e76786c81a2d49ba4ea9818500740527318d12f6ad51542963ec4`, separately linked exploratory executable differed due to paths/flags; both actually passed guest RC0. After action support was added, all **14** existing and new scripting/IPC/Objective-C tests were rebuilt with GCC16 and executed sequentially in the same AC090 guest, all RC0; full output: `tests/evidence/oolite-all14-original-script-objc-guest.log`.

**Caution:** The game receiver is the controlled selector-compatible test fixture `tests/oolite_objc_ship_mock.m`, *not* pinned `ShipEntity.m`. It **records** legacy action counts but does NOT execute Oolite's original legacy command language. No full Universe/OOJavaScriptEngine integration or complete playable game startup is claimed. Before production use, implement owner-only original action semantics including action arguments, ship lifecycle, errors and reentrancy, and dead-owner timeout handling.
