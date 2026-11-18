# Oolite ship guard semantics through GFOS JS, Exec and native GNU Objective-C

18 November 2026, AmigaOS 3.2.3 native 68040 in isolated AmigaChrome AC090 lab, GCC16, GFOS shared `javascript.library` and GFOpenGNUstep Foundation. This is selector-compatible test `ShipEntity`, *not* the full original Oolite `ShipEntity.m` or Universe.

`tests/oolite_js_shipentity_objc_guard_gate.c` sends three separate asynchronous GFOS JavaScript jobs through the game task's Exec IPC and generation-token ship registry, then invokes native ObjC selectors:

1. Set mock ship's `isPlayer=YES`; JavaScript setting `ship.maxEscorts=6` raises an exception (player ship uses the dedicated player API).
2. Set `isPlayer=NO` but mock escort group count to 5 (four existing escorts); JavaScript setting `ship.maxEscorts=3` is rejected and leaves the original 2 value untouched. This exercises the original `OOJSShip.m` *existing escort group* bound, not simply `MAX_ESCORTS`.
3. Set test mock native `OOCreditsQuantity` bounty to `2147483648ULL` (one above signed LONG max); JavaScript reading `ship.bounty` raises an exception rather than silently truncating a 64-bit native value.

All three cases were caught by JavaScript and yielded three `DONE` messages without deadlock. Test-only mock setters in `tests/oolite_objc_ship_mock.m` let the game task vary player role, escort group size and 64-bit bounty without altering actual Oolite source.

```
OOLITE_JS_OBJC_GUARDS player=1 group=1 bounty64=1 escorts=2 requests=3 complete=3 closed=1 status=PASS
OOLITE_OBJC_THREE_REG3_RC 0
```

`make js-objc-bridge js-objc-rules js-objc-guards` rebuilt all three native cross-language executables and **all three passed** in guest after adding the mock test controls; complete stdout `tests/evidence/oolite-js-objc-three-suite-guest.log`. `make js-objc-guards` output SHA-256 `ddc195cabc5791cd329da3e6dfaf1b222a37778dca5c46b5ee465d335eef1824` (differs from direct exploratory build due to flags/input). All three return RC0.

Production limitations: this deliberate signed-LONG JS scalar API refuses 64-bit values above INT32_MAX; a richer script number conversion is needed for full Oolite. Real game ship lifecycle and full title screen/gameplay not proven, and the Exec dispatcher still cannot recover if its owner task is dead or stalled.
