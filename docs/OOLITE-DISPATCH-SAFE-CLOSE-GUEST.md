# Exec game-task dispatcher: tracked in-flight messages and guarded shutdown

18 November 2026. Isolated AmiCradle AC090/AmigaOS 3.2.3, canonical GFOS `javascript.library`, GCC 16.2.

The prior Oolite experimental `OOGFOSGameDispatchClose` deleted the game-task's Exec `MsgPort` without checking whether other JavaScript tasks had already sent stack-backed requests whose replies were outstanding. That can leave worker calls waiting indefinitely or pointing at invalid port memory.

`OOGFOSGameDispatch` now counts `inFlight` requests and completions. `sendTracked` uses `Forbid` during the live-port check, increment, and `PutMsg`; `finishTracked` decrements under `Forbid` after the worker has collected its reply and destroyed its temporary reply port. Only the owner task can close; `OOGFOSGameDispatchTryClose()` returns 0 while any request is in flight and returns 1 after closing an idle port. The compatibility `Close()` wrapper calls `TryClose()` and never forcibly deletes a busy port. New requests after close fail, and requests sent by the game-task owner fail early rather than blocking it waiting for itself.

A two-JSTask guest gate, `tests/oolite_js_game_dispatch_safe_close_gate.c`, verifies that busy close is rejected, self-RPC is rejected, two workers still complete their requests and acknowledge completion, and idle close succeeds. Native guest test also rebuilt and reran existing single-task success, remote invalid-write, and two-client concurrency gates against the changed dispatcher source.

```
OOLITE_DISPATCH_REGRESSION_BEGIN
OOLITE_JS_GAME_DISPATCH ... status=PASS
OOLITE_DISPATCH_NORMAL_RC 0
OOLITE_JS_GAME_REJECT ... status=PASS
OOLITE_DISPATCH_REJECT_RC 0
OOLITE_GAME_TWO_JS_TASKS ... status=PASS
OOLITE_DISPATCH_TWO_TASKS_RC 0
OOLITE_GAME_SAFE_CLOSE ... busy_reject=1 owner_reject=1 closed=1 post_reject=1 status=PASS
OOLITE_DISPATCH_SAFE_CLOSE_RC 0
OOLITE_DISPATCH_REGRESSION_END
```

Full unabridged native output under `tests/evidence/oolite-dispatch-safe-close-regression-guest.log`. `make js-safe-close` rebuilt byte-for-byte equal to the separately compiled direct GCC binary, SHA256 `90f79f387b4a49f1a210f5a038722870e564639212f2d70ab5476ea7a885f716`.

**Not a bounded/cancellable IPC implementation.** A worker already in `WaitPort` still waits forever if the game owner never services that request. TryClose refuses to destroy the port, which prevents use-after-free but does not resolve stalled owner or missing completion. Completion messages are not proof all worker callbacks have returned: callers still must join `JS_Post` queues before destroying or freeing the containing dispatch structure. Reentrant game-to-JS calls, task death and actual Objective-C `ShipEntity` ownership remain to be handled before this code can replace Oolite's shipping scripting backend.
