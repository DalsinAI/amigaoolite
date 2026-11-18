# Oolite OOJSShip numeric properties over GFOS javascript.library — guest qualification

18 November 2026. Original property names and semantics referenced from pinned `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979/src/Core/Scripting/OOJSShip.m`.

`src/Core/OOGFOSShipProperties.c/.h` provide three actual OOJSShip JavaScript property names (`bounty`, `homeSystem`, `maxEscorts`) as GFOS `JSS_ACCESSOR` slots; plain `JSS_FIELD` is read-only and would not implement upstream setters. A caller-owned `OOGFOSShipHandle` directs callbacks through an explicit `getLong`/`setLong` backend; its validity guard refuses stale access by raising a JS exception rather than following the dead native ship pointer. Accessor callbacks happen in JavaScriptEngine's **separate task** and MUST NOT call Objective-C `ShipEntity` directly until bounded thread-safe game-task dispatch and entity lifetime control exist. This source is a narrow native adapter seam, not a complete `OOJSShip` replacement.

`tests/oolite_ship_properties_gate.c` uses a controlled C ship backend, enforces ranges, writes three properties from real JS, checks that backing values changed, invalidates the handle, and catches a stale-access exception.

Native AmigaChrome AC090 68040 guest passed both direct GCC16 build and independently rebuilt `make ship-properties` executable. The latter binary was not byte-for-byte identical to the direct compile (different build paths/flags); importantly, both actually returned RC 0 in the guest. Rebuilt executable SHA-256 `5408bffa1a6454a3739b10349f075fa320164195b1fdc5996d7031c61cd8fbbe`.

```
OOLITE_SHIP_PROPERTIES_REPRO_BEGIN
OOLITE_SHIP_PROPERTIES bound=1 bounty=44 home=42 escorts=4 writes=3 sum=90 stale_rc=0 stale=1 status=PASS
OOLITE_SHIP_PROPERTIES_REPRO_RC 0
OOLITE_SHIP_PROPERTIES_REPRO_END
```

Source, build target and full guest output are committed. No complete Oolite game startup is claimed. Open work: convert original ShipEntity lifetime and game-task dispatch into callback backend, include other numeric/string/vector properties with correct original setter behaviour, enforce concurrency/transaction semantics, then test during real Universe runtime.
