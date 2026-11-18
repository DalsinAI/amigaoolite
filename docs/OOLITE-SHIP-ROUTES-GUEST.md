# Per-ship native token routing in the Oolite GFOS JavaScript game-task dispatcher

18 November 2026. Target: native AmigaOS 3.2.3, AmiCradle AC090 68040; GFOS shared `javascript.library` with no SpiderMonkey. Pinned upstream Oolite: `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`.

The previous game-task dispatcher had a single `owner` ship. Every JS object received a pointer to that same dispatcher, making multiple concurrent ships indistinguishable. `OOGFOSGameDispatch` now supports a separate **routed** initialization mode: `OOGFOSGameDispatchInitRouted` takes a game-task-owned registry and a table of per-token get/set callbacks. Each script-exposed ship has a small caller-owned `OOGFOSGameShipProxy {dispatch, token}` stored in its `OOGFOSShipHandle`. When a property callback runs in `javascript.library`, it sends the token and requested property through the existing tracked Exec message port; it does *not* transfer `ShipEntity` pointers to the engine. Only the game task invokes its registry's lookup and game-object operations. Token zero is rejected. Old single-owner APIs remain valid and qualified.

The native `tests/oolite_js_game_dispatch_route_gate.c` exposes two distinguishable ship objects with tokens 1001 and 1002 to one JS task. The script changes ship A's bounty and ship B's homeSystem, reads both plus each ship's escort count, and checks the sum. The game-task registry then invalidates the first ship's token without deleting the JS object, and the second JS job catches that stale-token exception while successfully reading the second ship. Two queued JS jobs are joined before closure; eight property messages and two `DONE` completions are serviced by the game task. The invalidated token records one rejected lookup and no accidental write to another entity.

```
OOLITE_GAME_SHIP_ROUTES bound=1,1 jobs=0,0 complete=2 requests=8 writes=2 stale=1 wrong_task=0 first=91 second=43 closed=1 status=PASS
OOLITE_JS_ROUTES_RC 0
```

`make js-ship-routes` rebuilds byte-for-byte identical to separately compiled GCC 16.2 native AmigaOS program SHA-256 `a18ac8ed8c5185aee845b081c0d49911f2b0d7a0349188e6909e67d14b557769`. Full native guest output `tests/evidence/oolite-js-ship-route-suite-guest.log`, including **all six** earlier/revised IPC regression binaries returning code 0.

**Scope limits:** The game-task registry in this test is a controlled C fixture, not Objective-C `ShipEntity`. A production registry must issue non-reusable opaque tokens (or generation counters), manage retained/released entities and ship removals in the Oolite game task, and preserve upstream setters' game rules (e.g. `bounty` uses `setBounty:withReason:kOOLegalStatusReasonByScript`, `homeSystem` clamps negatives, and `maxEscorts` is constrained by existing escort count and `MAX_ESCORTS`; see pinned `src/Core/Scripting/OOJSShip.m`). As before, cancellation/timeouts for in-flight requests and reentrant callbacks remain unimplemented. Do not claim full-game integration or use this code as the live default until these are addressed.
