# Oolite GFOS game-task nonblocking dispatch poll (AC090 qualification)

18 November 2026. Introduced `OOGFOSGameDispatchPoll` as a frame-loop-safe alternative to `OOGFOSGameDispatchServe` for the Oolite game task. It calls `GetMsg` without `WaitPort`, returns 2 immediately when the queue is empty; 0 when one property request was handled; 1 for JavaScript-worker DONE; -1 for an invalid dispatcher, wrong owner task or malformed packet. Both entry points share the same request handler and preserve seal/queued rejection semantics. Blocking Serve remains for existing test runners.

This avoids blocking an Oolite frame merely because the JavaScript task has no messages. Production frame loop should process only a bounded number of messages per frame; no live Universe/main-loop integration is claimed yet. This is not a timeout for stalled client-side waits and does not solve abandoned worker recovery or borrowed stack-message lifetime.

`tests/oolite_js_game_dispatch_poll_gate.c` checks idle response (2) before JS work, six native JS property roundtrips through the game-task Exec queue, three native writes, owner-task isolation, worker DONE and RC 0.

Six independently rebuilt GCC 16.2 native 68040 programs ran in the dedicated AmigaChrome/AmigaOS 3.2.3 guest; all six returned code 0: `js-game-poll`, `js-game-dispatch`, `js-safe-close`, `js-seal-queued`, `js-ship-routes`, `js-token-table`. Full output: `tests/evidence/oolite-poll-six-regressions-guest.log`.

```
OOLITE_JS_GAME_POLL bound=1 post=0 completed=1 requests=6 writes=3 wrong_task=0 on_owner=0 sum=90 idle=2 processed=6 status=PASS
TEST_RC 0
```

Repro: `make js-game-poll GFOS_JS_SDK=<sdk> PREFIX=<toolchain> OUT=<folder>` using the existing repo's native SDK and cross toolchain; direct build and Makefile build binary hashes match SHA-256 `677a972f97a5d7653e8d7977cd2fcaad5cb11f1992d6fd60ee64203b1b11d20f`.

Open: game-frame integration and budget, bounded reply/cancellation, retained request ownership on timeout, actual executable startup. No playable Oolite claimed.
