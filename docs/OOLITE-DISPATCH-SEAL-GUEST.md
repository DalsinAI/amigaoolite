# Oolite game-task Exec dispatcher: sealed admission and safe completion

18 November 2026, native AC090 68040 AmigaOS 3.2.3 guest. The existing shared GFOS `javascript.library` runs all script jobs; there is no SpiderMonkey engine in this test.

`OOGFOSGameDispatchSeal` is an owner-game-task shutdown admission boundary. Under `Forbid()`, it marks the open dispatcher as sealed. Later `OOGFOS_DISPATCH_GET` or `SET` requests are refused at send time, while an existing `JS_Post` job may still send its `OOGFOS_DISPATCH_DONE` completion. All published messages are tracked as in-flight across the same `Forbid()`-guarded port lifetime protocol established in the previous `TryClose` change. The game task drains messages, joins its script-task jobs and then `TryClose` can dispose the idle port. The dispatcher cannot accidentally close with tracked messages in flight.

The new `tests/oolite_js_game_dispatch_seal_gate.c` runs a native guest JavaScript job that writes/reads three real Oolite-style numeric property names (six Exec RPCs), joins the job, seals the game task port, launches another JS job that expects a property read to throw, receives its `DONE` packet, joins it, and closes successfully. The second job's failed access did not touch any native ship property. The test verifies two completion messages and six total property requests, no wrong-task calls, and RC0.

```
OOLITE_JS_SEAL bound=1 post=0 completed=2 requests=6 writes=3 wrong_task=0 on_owner=0 first=90 second=1 seal=1 closed=1 status=PASS
OOLITE_JS_SEAL_RC 0
```

`make js-seal` builds the exact binary SHA-256 `4b0747f3bf7b509e1b205d98dbab7b6986027313df22512129ae4ef3fa9347ab`. All five existing and new IPC gate programs rebuilt and passed on the guest after this change. Full log: `tests/evidence/oolite-dispatch-seal-regression-guest.log`.

This does **not** implement timeouts or cancellation for already-sent requests, nor prevent a stalled game owner from indefinitely blocking a waiting JS task. A killed task or a reentrant call can still deadlock. Worker lifetime and handle ownership must be enforced by the caller before freeing the dispatcher. Real Objective-C `ShipEntity` adaptation remains pending; this gate uses a controlled C ship.
