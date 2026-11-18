# GFOS Oolite dispatcher: seal rejects already queued requests

18 November 2026, AC090 AmigaOS 3.2.3 native 68040 qualification.

## Problem and narrow change

`OOGFOSGameDispatchSeal` stops **new** GET/SET/ACTION packets in `sendTracked`. However an RPC queued immediately before `Seal` could still be served afterwards by `OOGFOSGameDispatchServe` and mutate an entity, unless the owner explicitly called `OOGFOSGameDispatchCancelQueued` first. This made `Seal` insufficient as an owner-state boundary when the normal `Serve` loop continued.

`OOGFOSGameDispatchServe` now rejects already-queued GET/SET/ACTION (and unknown non-DONE messages) with status 0 while `sealed`. It increments `cancelled`, replies to the caller's Exec reply port, and returns 0. DONE packets still reach the owner to allow script-worker shutdown to finish. The game-owner backend is never called for a rejected request.

## Deterministic guest test

`tests/oolite_js_game_dispatch_seal_queued_gate.c` posts an asynchronous original-style JavaScript `ship.bounty` access via `javascript.library`, waits for the RPC to enter the owner's Exec queue, calls `Seal` **before** the owner can `Serve` it, then calls `Serve` rather than `CancelQueued`. The JS exception is caught as result 1, its DONE message arrives and is acknowledged; a synchronous per-task JS_Post barrier joins the worker, and `TryClose` succeeds with no outstanding request. Native ship getter/write callback counters remain zero.

```
OOLITE_SEAL_QUEUED sealed=1 rejected=1 requests=0 owner_reads=0 writes=0 result=1 joined=0 close=1 status=PASS
OOLITE_SEAL_QUEUED_RC 0
```

## Seven executable regression batch

All recompiled against the amended dispatcher and run on the actual dedicated AmigaOS 3.2.3 / AC090 guest. Each returns **0**:

- `js-seal-queued` — sealed queued RPC rejected without invoking native backend
- `js-seal` — later accesses rejected, previously completed operations preserved
- `js-cancel` — two queued JS worker requests rejected and completed
- `js-safe-close` — cannot close with in-flight RPC or from wrong owner
- `js-game-dispatch` — normal unsealed property RPC flow unchanged
- `js-ship-routes` — routed ship identities, stale token handling
- `js-token-table` — generational token replacement and stale token rejection

Actual full output: `tests/evidence/oolite-seal-queued-seven-regressions-guest.log`. `make js-seal-queued` builds with native GCC16.2, `-m68040 -m68881 -noixemul`; independently built and Makefile-built test binaries matched SHA-256 `c19e468d713e9677ba3c11b2ed645d9f085e01d5c413cc8da014e8f1c1fd630f`.

## Remaining limitations

This closes an admission-ordering gap. It does not implement bounded waits, crash recovery if the game task disappears, prevention of reentrant deadlocks, or automatic shutdown of unjoined JavaScript jobs. Those must be solved and qualified before this experimental dispatcher becomes part of Oolite's production game loop. It is not evidence of a fully playable Oolite binary.
