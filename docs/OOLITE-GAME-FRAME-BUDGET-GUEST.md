# Oolite game frame dispatch budget: native 68040 qualification

18 November 2026, GFOS/AmigaOS 3.2.3 AC090 isolated lab.

`OOGFOSGameDispatchPumpFrame(dispatch,budget)` is an owner-task-only nonblocking loop over the previously qualified `OOGFOSGameDispatchPoll`. It processes **at most budget queued Exec messages** and returns the packet count; it returns 0 on an empty queue, and -1 on zero budget, wrong owner, invalid dispatcher or malformed packet. It increments `completions` for worker DONE notifications via the existing shared service implementation. Use a small bounded budget on each Oolite Universe frame; higher-level caller chooses frame budget. This prevents an unbounded backlog of JavaScript events from monopolizing the frame. The API does not create a separate JavaScript engine.

`tests/oolite_js_game_dispatch_frame_budget_gate.c` exercises a budget of one. The owner services six property operations and one worker completion across seven separate calls, preserving native ownership (zero wrong task accesses), correct writes and result 90. Empty queue returns 0; budget=0 rejects with -1.

```
OOLITE_JS_FRAME_BUDGET bound=1 post=0 completed=1 requests=6 writes=3 wrong_task=0 on_owner=0 sum=90 idle=2 empty_budget=0 zero_budget=-1 processed=7 status=PASS
TEST_RC 0
```

GCC 16.2.0 `-m68040 -m68881 -noixemul`, `make js-frame-budget`; direct and Makefile builds byte-identical SHA256 `c64fccd5c593b59c4fd602ac04f5b2b0310fceee362eb72c5496fb10548c6d60`.

Six rebuilt native guest test binaries all returned RC 0: `js-frame-budget`, `js-game-poll`, `js-safe-close`, `js-seal-queued`, `js-ship-routes`, `js-token-table`. Full raw output saved as `tests/evidence/oolite-frame-budget-six-regressions-guest.log`.

Limitations: the budget is message count, **not a wall-clock deadline**; a single slow owner callback could still stall the frame. Do not invoke on another task. This does not add bounded waiting/cancellation for a JavaScript sender whose owner stops responding, and cannot replace in-flight ownership handling or test against a playable Oolite engine. No full game launch claimed.
