# Oolite native ResourceManager built-in path guest gate

18 November 2026. Pinned upstream Oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

`patches/0003-gfos-resource-root.patch` adds an `OOLITE_GFOS` branch to the **original** `ResourceManager.m` `builtInPath` method. It returns `SYS:Oolite` by default or `OOLITE_RESOURCE_ROOT` when set, without modifying the existing non-GFOS logic. The patch applies cleanly to pinned upstream source. This patch does not yet make all of ResourceManager compilable.

`tests/oolite_resource_root_gate.m` compiles the patched `builtInPath` method **verbatim** into a standalone Objective-C `ResourceManager` harness class, then uses native GFOpenGNUstep/AmigaDOS to locate `Config` and load actual upstream `shipdata.plist` and `planetinfo.plist`. This isolates path discovery and Foundation property-list parsing from full ResourceManager dependencies (Universe, OXZ, scripting, rendering, logging and caches). It does not execute the complete upstream ResourceManager implementation.

## Native guest result

Native AC090 68040 AmigaChrome guest, dedicated isolated GFOS Foundation RC1 instance, 18 November 2026. GCC 16.2 `-m68040 -m68881 -noixemul`, native GFOpenGNUstep 1.31.1 static libraries, guest environment prepared with original Oolite Config assets mounted at `SYS:Oolite/Config`.

```
OOLITE_RESOURCE_ROOT_BEGIN
OOLITE_RESOURCE_ROOT root=SYS:Oolite dir=1 ship=194 planet=2050 status=PASS
OOLITE_RESOURCE_ROOT_RC 0
OOLITE_RESOURCE_ROOT_END
```

Native executable SHA-256: `aed4f44d7a6e702b6e74066014324ee7618f034232b472bf088e04c85b879311`. Guest staged binary and host-built binary hashes matched. Source: `tests/oolite_resource_root_gate.m`; guest stdout: `tests/evidence/oolite-resource-root-guest.log`.

## Next gates

1. Integrate this resource-root branch with the complete upstream `ResourceManager` without adding parallel manager code.
2. Wire `ResourceManager` path-list and configuration loading to the existing native `OOPListParsing` implementation; gate OXZ and caches on explicit capability rather than falsely claiming them implemented.
3. Resolve graphics, audio, NSRunLoop, SpiderMonkey and OXZ requirements for actual Oolite engine startup. No game window or full executable has launched yet.
