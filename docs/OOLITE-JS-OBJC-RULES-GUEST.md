# Native 68040 Oolite JavaScript-to-Objective-C property rule qualification

18 November 2026, isolated AmigaChrome AC090 native AmigaOS 3.2.3 guest, GCC 16.2. The canonical shared GFOS `javascript.library` evaluates scripts in its own Exec task. Property accessors send messages through `OOGFOSGameDispatch` to the game owner, which resolves a generation token and invokes Objective-C selectors through the native C/ObjC bridge and GFOpenGNUstep Foundation.

`tests/oolite_js_shipentity_objc_rules_gate.c` exercises property rules using the controlled selector-compatible `ShipEntity` class defined in `tests/oolite_objc_ship_mock.m`. JS writes `ship.bounty=-5`, `ship.homeSystem=-3`, and tries `ship.maxEscorts=17`. The upstream-compatible adapter clamps bounty and homeSystem to zero, sets original `kOOLegalStatusReasonByScript`, and rejects escorts > `MAX_ESCORTS` (16); no rejected write changes the native escort count. JS catches the error, and the final expression returns 3. Six property IPC operations are serviced, result code 0.

```
OOLITE_JS_OBJC_RULES bound=1 rpc=6 done=1 value=3 bounty=0 home=0 escorts=2 legal_reason=2 closed=1 status=PASS
OOLITE_OBJC_RULES_REPRO_RC 0
```

`make js-objc-rules` compiled/linked independently, and its native guest executable passed; SHA-256 `d84f3b5dd13d965253fb526da066bd5b5eee33c6a598deb9b62a45f02a3722bf`. The separately compiled initial executable passed too but had different bytes (different build commands/paths). Actual Makefile-run guest output in `tests/evidence/oolite-js-objc-rules-repro-guest.log`.

**Scope:** The test class has the original selected method signatures but is not original upstream `ShipEntity.m`/Universe. The full game does not yet link or run. Original `OOJSShip` methods and entity lifecycle must be integrated and validated before claiming playable Oolite.
