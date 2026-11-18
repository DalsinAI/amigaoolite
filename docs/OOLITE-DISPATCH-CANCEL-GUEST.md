# Oolite JS-to-game task IPC: queued request cancellation with replies

18 November 2026, AmigaOS 3.2.3 in isolated AmiCradle AC090 68040, native GFOS shared `javascript.library` task and GCC 16.2. This is a game-task-controlled cancellation transition, **not** a global watchdog or OS-level task termination.

`OOGFOSGameDispatchCancelQueued` is called only by the game-owner task. First it seals the dispatch port under the existing tracked admission protocol so no additional `GET` or `SET` can be enqueued. Then it drains already queued Exec messages. It replies with a failure status to property messages, returning their stack-backed message storage to waiting JavaScript tasks; queued `DONE` messages are accounted as completions and acknowledged normally. The script engine can catch the rejected property access as a JavaScript exception and deliver a `DONE` completion over the same still-open port. After receiving/acknowledging completions and joining all JS tasks, the owner may safely call `TryClose` once in-flight reaches zero.

`tests/oolite_js_game_dispatch_cancel_gate.c` schedules two independent GFOS JavaScript tasks, each trying to read a native ship property. The game task waits until at least one request has arrived and cancels queued work before serving any native getter. Both workers recover via JS exception handlers, successfully send completion packets, and are synchronously joined before closure.

```
OOLITE_GAME_CANCEL_QUEUED bound=1,1 posts=0,0 complete=2 rpc=0 writes=0 wrong_task=0 same_task=0,0 values=1,1 joined=0,0 done=1,1 cancelled=2 closed=1 status=PASS
OOLITE_JS_CANCEL_RC 0
```

Exact GCC 16.2 guest executable rebuild SHA-256 `3347937cea34941b894ae4989ae03c9a495d84144d5ec795ccda270fefb8b20a`. All seven game-task IPC regression gates compiled and **passed in native guest** against this revision. Complete output: `tests/evidence/oolite-game-task-cancel-seven-guest.log`.

**Remaining limitations:** This cannot unblock a worker if the owner game task crashes, stops running entirely, or is blocked inside a native getter/setter. There are no timer.device deadlines or independently owned heap packets, so forcibly dropping a waiting stack-backed request remains unsafe. The caller must still join JavaScript job queues and keep the dispatcher/proxies alive until all callbacks finish. Tests use controlled C ships and do not verify real Oolite Objective-C `ShipEntity` objects, full-game startup or gameplay.
