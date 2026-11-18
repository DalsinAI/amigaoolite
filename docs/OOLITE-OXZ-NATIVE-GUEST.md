# Oolite upstream OOOXZManager — first native GFOS qualification

18 November 2026. Pinned OoliteProject/oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

`patches/0004-gfos-oxz-manager-paths.patch` modifies the **original** `OOOXZManager.m` with `OOLITE_GFOS` branches, retaining all upstream implementations on other platforms. It preserves the three public methods `installPath`, `extractAddOnsPath` and `additionalAddOnsPaths`, mapping their defaults to `SYS:Oolite/ManagedAddOns`, `SYS:Oolite/AddOns` and an empty list. Explicit `OO_MANAGEDADDONSDIR`, `OO_ADDONSEXTRACTDIR` and comma-separated `OO_ADDITIONALADDONSDIRS` remain supported using C `getenv` rather than SDL. Patch replay against pristine upstream source passed. No SDK or Team 4 changes.

`tests/oolite_oxz_paths_gate.m` embeds the three patched original upstream methods verbatim into an isolated Objective-C class for path qualification, since the full OOOXZManager currently depends on game GUI, OXZ index/network, cache, script, Universe and MiniZip code. It **does not execute the complete manager**, nor test overrides, archive extraction, manifest validation, installation or downloads.

## Native execution

GCC 16.2 `-m68040 -m68881 -noixemul` with GFOpenGNUstep 1.31.1 archives. Isolated dedicated AmigaChrome AC090 68040 guest, Kickstart/Workbench 47.2/47.100.

```
OOLITE_OXZ_GUEST_BEGIN
OOLITE_OXZ_PATHS managed=SYS:Oolite/ManagedAddOns addon=SYS:Oolite/AddOns extra=0 status=PASS
OOLITE_OXZ_GUEST_RC 0
OOLITE_OXZ_GUEST_END
```

Native binary SHA-256: `88fb3098701b3b47285900465b7338c09dc7d038deb026686909a10efaa0eee6`.

## Remaining work

1. Bring upstream MiniZip ZIP64 reader into the native 68k toolchain with proper AmigaDOS file access, including malformed-archive and path-traversal protection for extraction.
2. Compile original manager class against native GFOpenGNUstep, gating actual GUI/network dependencies as appropriate, without creating a second OXZ implementation.
3. Verify real OXZ manifest enumeration, installation/extraction and failure recovery in guest. Only then enable OXZ search paths in Oolite ResourceManager.
4. Download/update functionality remains dependent on the GFOS networking/TLS and UI integration; do not call it complete yet.
