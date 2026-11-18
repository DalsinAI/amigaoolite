# Oolite multi-client JS-to-game Exec IPC qualification

18 November 2026. Native AmigaOS 3.2.3 AC090 68040 dedicated lab, canonical GFOS `javascript.library`, GCC 16.2.

`tests/oolite_js_game_dispatch_two_tasks_gate.c` opens **two separate JSTasks** with their own JavaScript global objects, and schedules two `JS_Post(..., wait=0)` jobs. Both expose a controlled C ship through the same `OOGFOSGameDispatch` backed by the game owner's private Exec message port. One task sets/reads bounty=44, another sets/reads homeSystem=42. The game task services four RPCs and two completion messages, checks every native getter/setter runs only on its owning Exec task, then issues a synchronous no-op JS_Post barrier to each independent JS task before reading final job acknowledgments. These barriers are essential: receiving the DONE Exec message does not guarantee the worker has received its `ReplyMsg` and returned from `OOGFOSGameDispatchDone`.

```
OOLITE_TWO_JS_JOIN_BEGIN
OOLITE_GAME_TWO_JS_TASKS bound=1,1 posts=0,0 complete=2 rpc=4 writes=2 wrong_task=0 same_task=0,0 values=44,42 joined=0,0 done=1,1 status=PASS
OOLITE_TWO_JS_JOIN_RC 0
OOLITE_TWO_JS_JOIN_END
```

`make js-two-tasks` reproduces the exact binary SHA-256 `d31a5975b69f2fb9393f31b66e0ec36a2a7d0611a700e10eebf3455c51701e07` (matches independently compiled test). Actual guest output: `tests/evidence/oolite-js-two-tasks-guest.log`.

The first guest run completed both jobs and four property RPCs, but failed its final `doneOK` check because the game owner inspected the flags before the workers finished their acknowledgements. The synchronous JS_Post barrier solved that test-harness race; no engine API or shipped IPC implementation was modified.

**Unresolved**: `OOGFOSGameDispatchClose` remains unsafe if a client can still submit or wait on an Exec message (the explicit caller contract is join first, close second). There is no bounded wait, shutdown cancellation, backpressure, nested game-to-JS reentrant call handling, or real Objective-C `ShipEntity` adapter. Do not use this prototype as the live full-game dispatcher until those are qualified. This gate proves only a two-client concurrent success path, not lifecycle safety under failure.
