# Oolite upstream MiniZip: native OXZ guest qualification

18 November 2026; pinned OoliteProject/oolite source commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

Oolite bundles its own MiniZip `ioapi.c`, `ioapi.h`, `unzip.c` and `unzip.h`. `patches/0005-gfos-minizip-offset-adapter.patch` adds a narrowly guarded `OOLITE_GFOS` adapter for target toolchains lacking `fopen64`, `ftello64` and `fseeko64`. The adapter rejects offsets greater than `LONG_MAX` in the guest's signed 32-bit stdio interface. It is NOT a claim of transparent >2GB/ZIP64 file support. The original non-GFOS path is unchanged.

Both unmodified upstream `unzip.c` and patched `ioapi.c` cross-compiled successfully with GCC 16.2 `-m68040 -m68881 -noixemul` against the existing guest zlib. No competing ZIP library was introduced.

`tests/oolite_oxz_zip_gate.c` uses Oolite's upstream `unzOpen64`, `unzGoToFirstFile`, `unzGetCurrentFileInfo64`, `unzOpenCurrentFile` and `unzReadCurrentFile` to open `SYS:Oolite/fixture.oxz`, locate `manifest.plist`, decompress it and check its controlled manifest contents. The fixture is a synthetic, single-entry ZIP archive, not an actual published OXZ expansion. It is provided at `tests/fixtures/oolite-oxz-fixture.oxz` with SHA-256 `15094c5cf2746cc7756312aa5bb1a14f7c80c041fd1ad064f2fd41e50300374c` and must be staged as `SYS:Oolite/fixture.oxz`.

## Native AmigaChrome result

Dedicated AmigaChrome AC090 68040 guest, with original guest zlib and AmigaDOS filesystem:

```
OOLITE_OXZ_ZIP_BEGIN
OOLITE_OXZ_ZIP path=SYS:Oolite/fixture.oxz entry=manifest.plist bytes=53 status=PASS
OOLITE_OXZ_ZIP_RC 0
OOLITE_OXZ_ZIP_END
```

Cross-built binary SHA-256: `2188cce3998b8f78f4967c264e3dd72060940cd64d91f656f306452b01b8933a`. Actual guest output is preserved in `tests/evidence/oolite-oxz-zip-guest.log`.

## Not yet proven

- Full `OOOXZManager` Objective-C compilation and runtime; this only tests its existing MiniZip dependency and separately the three path methods.
- Parsing real-world OXZ manifests, installing/renaming/extracting directories, transaction rollback or cleanup.
- ZIP path traversal, duplicate names, malicious/malformed archives, size bombs, symlinks, CRC failure and capacity exhaustion.
- Network download, package-index fetch, update checking, GUI and dependency resolution.

**Do not enable automatic OXZ installation or arbitrary-archive extraction until these safety and runtime gates pass.** Continue using the pinned upstream library and original Oolite manager APIs.
