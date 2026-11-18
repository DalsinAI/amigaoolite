# Oolite GFOS game-task ship identity: O(1) generation-token table

18 November 2026. In the native AmigaOS 3.2.3 AC090 68040 guest, the shared GFOS JavaScript script-task communicates with Oolite's owner game task via the existing Exec IPC dispatcher. No original `ShipEntity` instances are constructed in these tests.

`OOGFOSShipTokens.{c,h}` is a fixed 4,095-slot game-task-owned table. The lower 12 token bits encode slot index +1; the high 20 bits encode generation. Lookup and remove are O(1) via the slot field and a full 32-bit token comparison. Only the original game task can mutate or resolve a token; the `javascript.library` task sees an opaque integer token through `OOGFOSGameShipProxy`, not a game-owned pointer. When a ship leaves, its slot becomes empty. Reusing the slot increments generation; existing JS wrappers holding its old token never resolve to the replacement. A slot with its 20-bit generation exhausted is retired permanently rather than allowing an old token to alias a new ship. This is a bounded initial registry and needs runtime capacity measurement against real Oolite universes.

Native test `tests/oolite_js_game_dispatch_token_table_gate.c` exposes ship A and B, writes/reads their separate native properties through the game task, removes ship A, **forces reuse of the same slot** for new ship C, and confirms old ship A cannot resolve while B and C still can. `tests/oolite_ship_token_capacity_gate.c` fills all 4095 slots, checks an extra allocation fails, replaces an earlier ship in the vacated slot with generation2, and confirms generation exhaustion retires a slot. All accesses are owner-task-guarded.

```
OOLITE_GAME_SHIP_TOKENS ... old=4097 new=8193 stale=1 wrong_task=0 status=PASS
OOLITE_JS_TOKEN_TABLE_RC 0
OOLITE_SHIP_TOKEN_CAPACITY slots=4095 full_rejected=1 old=4097 replacement=8193 retired=1 status=PASS
OOLITE_TOKEN_CAPACITY_RC 0
```

`make js-token-table` SHA-256 `d5104bdd51f67f65c23887ebab8143d06688ace8f91cdac92d9f51037138ad9f`, `make js-token-capacity` SHA-256 `b78279000221ac3fd73c22f842d1d7fb47c05f8579454eba0f51ba44825c68dd`; both byte-identical to direct GCC 16.2 builds. **All nine** Oolite game-task dispatcher/token regressions compiled and executed with exit code 0 on the native guest. Full log `tests/evidence/oolite-nine-ipc-token-suite-guest.log`.

This table owns only the opaque token mappings, not game objects; production must add retained/released `ShipEntity` lifetimes and exact original Objective-C getter/setter semantics. Existing Exec request cancellation only applies while the game-owner task can run; no independent watchdog or bounded timeout exists. Full Oolite game startup, data loading, rendering and gameplay remain unqualified.
