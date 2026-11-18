# Oolite GFOS JavaScript-to-game-task Exec IPC qualification

18 November 2026, 12:20-12:23 UTC. `DalsinAI/amigaoolite` development, pinned upstream `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`.

## Purpose and actual implementation

`src/Core/OOGFOSGameDispatch.{c,h}` is an **experimental** private AmigaOS Exec message-port bridge between the already qualified shared `javascript.library`/JavaScriptCore task and Oolite's game-task owner. It does **not** embed a JavaScript engine, create another process, or link SpiderMonkey. Each `OOGFOSShipProperties` get/set callback uses an owned Exec reply port, a fixed-size message and `PutMsg`/`WaitPort`/`ReplyMsg`; only the owner game task services the request. JS execution is started with the canonical asynchronous `JS_Post(..., wait=0)`. The game task pumps messages until a matching completion packet, then closes the JS task and its private port. The bridge checks `FindTask(NULL)` before servicing/closing its owner port.

The initial script changes `ship.bounty`, `ship.homeSystem`, `ship.maxEscorts` and reads them back. Six script-to-game property operations are serviced, three native writes, all by the actual game owner task. The script runs on a distinct JavaScript task. The owner backend is still a controlled C ship, **not** original Objective-C `ShipEntity`.

## Guest evidence

`make js-game-dispatch` produces a native GCC 16.2 `-m68040 -m68881 -noixemul` AmigaOS executable using the canonical GFOpenSDK `javascript.library` vector headers. Independently rebuilt and retested after adding the game-task-only `Serve`/`Close` checks. Latest native executable SHA-256 `260704e096b00009e3f7ec14756688b139c15821aa0aacf4ad77da18b48e3731`.

The isolated AC090 AmigaChrome guest was restarted after an unrelated overnight emulator exit. The first IPC guest run and then owner-task-check regression returned RC 0:

```
OOLITE_JS_GAME_OWNER_BEGIN
OOLITE_JS_GAME_DISPATCH bound=1 post=0 completed=1 requests=6 writes=3 wrong_task=0 on_owner=0 sum=90 status=PASS
OOLITE_JS_GAME_OWNER_RC 0
OOLITE_JS_GAME_OWNER_END
```

Full saved output: `tests/evidence/oolite-js-game-dispatch-guest.log`. This is native guest execution, not a host-side simulation.

## Integration and safety limits — not production-ready yet

- The IPC request/reply waits are **unbounded**. If the game loop is blocked, shutting down, not pumping its port, or if the script task is cancelled mid-request, the caller can deadlock. Add timeout/cancellation and in-flight ownership accounting before using with a live game.
- The temporary JS callback stores a borrowed native handle whose ownership must be enforced. `OOGFOSShipInvalidate` alone is not cross-task lifetime coordination. Ship removal needs game-task invalidation and request ordering, plus a stable handle table/generation ID.
- Do not call original Objective-C `ShipEntity` methods from the JavaScriptEngine task; use the owner-side backend, with correct retain/release semantics and stale-entity checks. Work remains to connect actual `ShipEntity` to the backend.
- The generic reply port is dynamically created per request. This is a functional prototype, not yet measured or batched; check high-frequency script callback overhead.
- The test drives one JS_Post job; concurrency, nested callbacks, engine shutdown, exception propagation, cancellation and multi-ship transactions have not been qualified.
- No full Oolite engine binary, title screen or gameplay has been built/tested by this gate.

Next: add bounded cancellation and controlled shutdown, then integrate actual `ShipEntity` getters/setters under game-task ownership, and test real Oolite scripts using those objects. Do not merge this experimental bridge as the engine's default path until those conditions are met.
