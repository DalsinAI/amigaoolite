# Oolite OXZ native guest extraction with rollback — tested

18 November 2026; pinned upstream OoliteProject/oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

## Purpose and ownership

`src/Core/OOOXZGFOSExtraction.m` and `.h` are a GFOS filesystem extraction **backend**, not an alternate OXZ manager. The original upstream `OOOXZManager` remains authoritative. Patch `0008-gfos-oxz-native-extraction.patch` applies **after** `0004`, `0006` and `0007` and routes the manager's `extractOXZ:` method through `OOGFOSExtractOXZ` only for `OOLITE_GFOS`, preserving non-GFOS extraction logic and existing public APIs. It also selects `extractAddOnsPath` as the GFOS native extraction root instead of depending on the unported user-root initialization. That upstream manager patch has replay/source validation; **the whole manager class has not been linked/run yet**.

The helper uses pinned upstream Oolite MiniZip plus `OOOXZArchivePreflight` and `OOOXZPathSafety` to validate a package before any writing, and rechecks member names before each write. It streams contents through guest stdio, enforces expected byte counts and MiniZip CRC, writes to a unique `.partial` directory and then renames it into place. `mkdir`/`rmdir`/`rename` work with native AmigaDOS paths on this guest. A bounded native POSIX tree-removal routine cleans partially extracted files/directories on failure, and refuses to recursively follow symlinked directories.

This path was necessary because GNUstep `NSFileManager createDirectoryAtPath:withIntermediateDirectories:` created a folder on the ACFS-backed host but hung without returning in an earlier 68040 guest test. Those stalled runs were shut down, and their scripts/logs and abandoned partial directories were archived under the dedicated test lab diagnostics, not silently deleted. The problem should be tracked as a separate GFOpenGNUstep/ACFS investigation.

## Native 68040 guest evidence

`tests/oolite_oxz_rollback_gate.m` ran in the dedicated AC090 68040 AmigaChrome guest with the canonical GNUstep Foundation libraries and original Oolite MiniZip. It installed a controlled, valid OXZ to `SYS:Oolite/AddOns/GFOSRollbackProbe.oxp`, read back its 53-byte `manifest.plist`, and rejected a manipulated CRC-failure OXZ and a traversal-entry OXZ without leaving those destinations or their staging directories behind.

```
OOLITE_OXZ_ROLLBACK_BEGIN
OOLITE_OXZ_ROLLBACK installed=1 bytes=53 crc_rejected=1 crc_clean=1 traversal_rejected=1 traversal_clean=1 status=PASS
OOLITE_OXZ_ROLLBACK_RC 0
OOLITE_OXZ_ROLLBACK_END
```

Tested binary SHA-256: `3aeeb9c06bed76bcd29bc073a34d29e2ed0124ee1b3672c61ea16c46ce5a451d`. Exact guest stdout: `tests/evidence/oolite-oxz-rollback-guest.log`. Rebuilding using `make oxz-extract UPSTREAM=<pinned source with 0005 applied> PREFIX=<GCC16 prefix>` produced a **byte-for-byte identical executable**. The CRC-bad fixture is committed at `tests/fixtures/oolite-oxz-crc-bad.oxz` and has intentionally invalid CRC. It must be staged as `SYS:Oolite/crc-bad.oxz`; the existing valid and traversal fixtures are also required.

## Still not qualified

The entire original OOOXZManager class, GUI/download pipeline, real-world OXZ packages, dependency manifests and installation metadata, package signature/trust policy, all exceptional storage failures, bounded decompression time, symlink races/TOCTOU on mutable host folders, and installed-pack lifecycle remain incomplete. Do not enable unattended arbitrary OXZ installation or call the full game playable from these component results.
