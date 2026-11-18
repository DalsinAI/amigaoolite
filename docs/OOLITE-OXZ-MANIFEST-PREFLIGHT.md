# Required OXZ archive root manifest — native 68040 preflight

18 November 2026. `OOOXZArchivePreflight.c` now rejects archives without an unambiguous root `manifest.plist`, in addition to earlier path traversal, duplicate-member, symlink, entry-count and decompression-budget checks. This is archive-structure validation, **not** validation of a manifest's required properties or Oolite version compatibility.

Actual native AmigaChrome AC090 68040 guest test, rebuilt from repository `make oxz-preflight` target against pinned upstream Oolite MiniZip with patch 0005:

```
OOLITE_OXZ_PREFLIGHT_MANIFEST_BEGIN
OOLITE_OXZ_PREFLIGHT valid=1 entries=1 missing_rejected=1 escape_rejected=1 duplicate_rejected=1 missing_manifest_rejected=1 status=PASS
OOLITE_OXZ_PREFLIGHT_MANIFEST_RC 0
OOLITE_OXZ_PREFLIGHT_MANIFEST_END
```

SHA-256 of binary: `9c682262b1910524fc13f9755e80481ac2a527afcf94abbfac093550d39eab42`. New controlled negative test archive: `tests/fixtures/oolite-oxz-no-manifest.oxz`, staged in the guest as `SYS:Oolite/no-manifest.oxz`.

The pinned upstream `ResourceManager validateManifest:` additionally requires string fields `identifier`, `version`, `required_oolite_version` and `title`, and then performs version compatibility and duplicate-identifier checks. Earlier minimal OXZ fixtures only test ZIP and parser mechanics; they do not represent fully valid loadable Oolite expansions. A separate manifest property/schema gate is required before arbitrary installations are enabled.
