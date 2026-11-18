# AmigaOolite: Foundation resource access on AmigaOS

18 November 2026: Native GCC 16.2 / GFOpenGNUstep Base 1.31.1 guest test on isolated AmigaChrome AC090 68040, Kickstart 47.2, Workbench 47.100.

The first probe looked for DH1:OoliteData/oolite-probe.txt and failed (rc2), because guest Info confirmed only DH0/System and RAM were mounted; the presence of a host-side DH1 directory did not mount it. The actual mounted SYS:ACAgent/oolite-probe.txt returned YES via NSFileManager fileExistsAtPath: and 25 bytes via NSData dataWithContentsOfFile:. The first SYS probe used an incorrect hardcoded 24-byte expectation (rc2). The final fixture checks its length using strlen.

The Makefile's `resource-probe` target cross-built an AmigaOS loadseg executable, SHA256 e16da196888289a6a6a319dcc8ebabfe2c409800fdbb592d6dd7e50c06cd616a. That build was executed in the guest and returned:

```
OOLITE_RESOURCE_REPRO_BEGIN
RESOURCE_00_START
RESOURCE_01_STRING
RESOURCE_02_FOUND 1
RESOURCE_03_DATA 25
OOLITE_RESOURCE_PASS
OOLITE_RESOURCE_REPRO_RC 0
OOLITE_RESOURCE_REPRO_END
```

Build from repository `make resource-probe` with native GFOS Base/library variables configured; stage `tests/fixtures/oolite-probe.txt` in the guest at SYS:ACAgent/oolite-probe.txt. Source: `tests/oolite_resource_probe.m`. Raw guest stdout: `tests/evidence/oolite-resource-68040-guest.log`.

This validates Foundation read-only file access on a mounted AmigaDOS volume. It is NOT upstream Oolite ResourceManager startup. Next: mount and validate the actual Oolite resource tree, directory traversal, and upstream ResourceManager modules; then work on GFOpenGPU, input, audio and scripting while waiting on Team 4's multicore SDK updates.
