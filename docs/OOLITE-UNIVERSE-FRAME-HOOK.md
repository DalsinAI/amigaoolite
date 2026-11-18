# Optional GFOS Oolite Universe game-frame hook

18 November 2026. Pinned source: `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`.

The original `src/Core/Universe.m` has `-[Universe update:(OOTimeDelta)inDeltaT]` at line 6421 of the pinned revision. `patches/oolite/0001-gfos-universe-frame-pump.patch` adds an **opt-in** `OOGFOSGameFrameUpdate()` call immediately on entering this original method, guarded by `#if defined(OOLITE_GFOS_GAME_DISPATCH)`. The patch was checked with `git apply --check` against the exact pinned upstream tree (PASS), without modifying the upstream checkout. This is a source patch/integration seam; full `Universe.m` compilation and playable game startup are not claimed.

`src/Core/OOGFOSGameFrame.{c,h}` implement the owner-task-only binding between the current `OOGFOSGameDispatch` and that hook. When no dispatcher is bound, game frames do no IPC work. Owner task installs with a nonzero message budget; `OOGFOSGameFrameUpdate` pumps at most that many Exec messages via `OOGFOSGameDispatchPumpFrame`. The same owner must unbind, join JavaScript worker tasks, seal/cancel outstanding operations and close the dispatcher before freeing it. Wrong-owner bind/unbind/update is rejected; replacing an active dispatcher without unbinding is rejected.

The dedicated AmigaChrome AC090 AmigaOS 3.2.3 68040 guest executed the original-style JS property test via this new hook, with one packet per update and seven calls for six RPCs plus worker DONE. The native program, linked using canonical `javascript.library`, returned RC0:

```
OOLITE_JS_FRAME_HOOK bound=1 post=0 completed=1 requests=6 writes=3 wrong_task=0 on_owner=0 sum=90 idle=2 empty_budget=0 zero_budget=-1 processed=7 installed=1 uninstalled=1 idle_frame=0 foreign_frame=-1 foreign_unbind=0 status=PASS
OOLITE_FRAME_HOOK_OWNER_RC 0
```

`make js-game-frame-hook` and direct GCC 16.2.0 builds are byte-identical, SHA256 `602fbe7233526801e432b0d3b38e8cafc0325f7ab27013b636fea1ad4c58b02e`. Raw guest output: `tests/evidence/oolite-game-frame-hook-guest.log`.

Remaining blockers include full Oolite dependency build, actual `Universe` runtime, startup of GFOS scripting for live ship instances, renderer/audio integration, and bounded client-side waits. The message budget is a **count**, not a wall-time limit. A long-running callback may still delay the frame. The hook is OFF by default and must not be enabled for production until proper lifecycle management and the complete game build pass.

The requalified guest test also invokes frame update and unbind from the **foreign JS task**: both refuse the operation (`foreign_frame=-1`, `foreign_unbind=0`) while the game-task owner remains correct. This is a guard test, not a claim of cross-task lifetime safety if a game owner crashes.
