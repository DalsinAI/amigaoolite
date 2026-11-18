# Two independent native Oolite original ship scripts and game-owner Objective-C callbacks

18 November 2026. Unmodified original `oolite-default-ship-script.js` (3757 bytes from pinned OoliteProject upstream, staged at `SYS:Oolite/Scripts/`), canonical GFOS `javascript.library`/JavaScriptCore, native AmigaOS 3.2.3 on AC090 68040 and GFOpenGNUstep Foundation. No embedded SpiderMonkey.

`tests/oolite_js_two_original_ship_objc_gate.c` creates two **independent JSTasks**, each with a distinct JavaScript global object and a distinct game-task generation token mapped to a different controlled Objective-C ship fixture. Both run the same original Oolite script asynchronously with different legacy action lists. They share one game-owner Exec message port. The game's action dispatch resolves each token and calls its corresponding mock Objective-C receiver, recording three method calls and four action-list entries on ship A, and three calls and five entries on ship B. Each script verifies its one-shot `shipSpawned` handler deletion and inherited `ship.bounty` host property through the game owner; eight IPC requests, two `DONE` messages, both workers explicitly joined before unregistering the game-owned ship references.

```
OOLITE_TWO_ORIGINAL_JS bytes=3757 jobs=0,0 done=2 rpc=8 calls=3,3 actions=4,5 results=1,1 closed=1 status=PASS
OOLITE_TWO_ORIGINAL_REBUILT_RC 0
```

`make js-two-original-objc` builds the GCC 16.2 native 68040 test executable, which independently passed in the dedicated AC090 guest together with `make js-original-objc-event` (both returned RC0). Rebuilt binary SHA-256 `8af71e17844756d880cd7ebe5fb032e16aaf880d9f2888cb7e83e252cea912cc`; exploratory direct build differed due to build path/flags. Full guest output: `tests/evidence/oolite-two-original-objc-repro-guest.log`.

**Scope limitation:** This proves independent JavaScript task/ObjC mock isolation but is **not** an appropriate production scaling model for Oolite's many ships: each `JSTask` owns an independent heap and default ~512 KiB stack. Real engine should share a bounded number of JS tasks and isolate per-ship script state as JS objects on the task. The next task is testing exactly that. Objective-C ship is a controlled selector-compatible fixture, not original `ShipEntity.m`; legacy command strings are counted, not executed.
