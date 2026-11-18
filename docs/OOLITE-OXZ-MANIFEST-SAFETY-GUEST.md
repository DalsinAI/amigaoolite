# Oolite OXZ manifest and extraction-path validation — 68040 guest

18 November 2026. Pinned upstream Oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

## OXZ archive manifest decoding

`tests/oolite_oxz_manifest_gate.m` combines the **original Oolite MiniZip unzip/ioapi** with GFOpenGNUstep `NSPropertyListSerialization`. The test opens the committed controlled `tests/fixtures/oolite-oxz-fixture.oxz`, finds `manifest.plist`, limits the uncompressed manifest allocation to 1 MiB, decompresses the entry, verifies its CRC via MiniZip close, parses the OpenStep property list and validates the `identifier` and `version` keys. This is a standalone integration test, not the complete upstream `OOOXZManager` runtime.

Native guest result (`tests/evidence/oolite-oxz-manifest-guest.log`):

```
OOLITE_OXZ_MANIFEST_BEGIN
OOLITE_OXZ_MANIFEST archive=1 bytes=53 identifier=org.oolite.test version=1.0 status=PASS
OOLITE_OXZ_MANIFEST_RC 0
OOLITE_OXZ_MANIFEST_END
```

68040 executable SHA-256 `f587e44158c18c63e3aba40f4deda13bcd944a68110fd88964a677ec2e5b1c24`.

## Extraction path safety

`src/Core/OOOXZPathSafety.h` provides the GFOS-only `OOGFOSSafeOXZEntry` validator. `patches/0006-gfos-oxz-extraction-safety.patch` applies **after** `0004-gfos-oxz-manager-paths.patch` to the original upstream `OOOXZManager.m`. The guard verifies the full untruncated ZIP filename before any directory or file creation. It rejects absolute paths, AmigaDOS volume prefixes (`:`), backslashes, embedded controls or NUL, empty interior segments, `.` / `..` segments and names at or above the current 512-byte buffer cap. It allows normal relative paths and a trailing slash for directories.

The 22-case `tests/oolite_oxz_path_safety_gate.c` passed with host AddressSanitizer+UndefinedBehaviorSanitizer and in native AC090 68040 AmigaChrome:

```
OOLITE_OXZ_SAFETY_BEGIN
OOLITE_OXZ_SAFETY cases=22 failed=0 status=PASS
OOLITE_OXZ_SAFETY_RC 0
OOLITE_OXZ_SAFETY_END
```

68040 executable SHA-256 `dca10826536ad77eda453dc46119c5e868ea154843108a05289eb747eabdeed3`.

## Unqualified and important restrictions

This patch does not yet establish safe installation. Full original manager linking, **real-world** archive imports, symlink/duplicate-path handling, atomic write/rollback, decompression budget, malicious ZIP64 metadata, manifest schema validation, install/uninstall and network operation remain unproven. Automatic installation must remain disabled until these tests pass. No changes to Team 4 SDK.
