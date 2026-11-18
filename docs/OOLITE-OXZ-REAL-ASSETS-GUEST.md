# Native OXZ extraction of original Oolite ship definitions

18 November 2026. Full upstream Oolite source pinned at `21978c37b2f11c8664adf3e2062efc1d0c875979`.

The controlled test package contains two entries: a synthetic Oolite four-field manifest for qualification and the **unaltered upstream `Resources/Config/shipdata.plist`** from the pinned Oolite repository. The latter is 128,966 bytes and has SHA256 `f4910d79b88cc803f7bd785fd2fa7d34597c0c891925659c23748313052ac166`. The ZIP fixture is assembled from the pinned source for tests; do not vendor upstream assets into the port overlay. The temporary test archive SHA256 is `50adb5402dd974aeace7892eb86d0df375fdb5f15782de4d111151d36ee4adef`; stage in guest as `SYS:Oolite/real-assets.oxz`.

`tests/oolite_oxz_real_assets_gate.m` uses `OOGFOSExtractOXZ` with original upstream Oolite MiniZip, its archive preflight and GFOS filesystem backend. After extraction to the dedicated guest's `SYS:Oolite/AddOns/GFOSRealAssetsProbe.oxp`, it loads `Config/shipdata.plist` via native GFOpenGNUstep `NSData` and `NSPropertyListSerialization`, verifies OpenStep format and checks exactly 194 ship definitions.

Actual AmigaChrome AC090 68040 guest, GCC 16.2:

```
OOLITE_OXZ_REAL_ASSETS_BEGIN
OOLITE_OXZ_REAL_ASSETS extracted=1 bytes=128966 ships=194 format=1 status=PASS
OOLITE_OXZ_REAL_ASSETS_RC 0
OOLITE_OXZ_REAL_ASSETS_END
```

Native binary SHA256 `c0098df2ef6bce62330f1ae888cf9ed78a124d107b9b77fd2e2eab5caaf4f40a`.

Qualification is of native extraction and Foundation data loading, not gameplay. Full OOOXZManager itself has not linked. Neither version compatibility nor dependency resolution nor user-facing install/update UI is validated.

## Rebuilt-current-preflight qualification

After requiring a root manifest in `OOOXZArchivePreflight.c`, the full `make oxz-real-assets` target was rebuilt and tested afresh in the same native 68040 guest. The test destination was changed to `SYS:Oolite/AddOns/GFOSRealAssetsRepro.oxp` to avoid overwriting its earlier successful installation. The rebuilt executable SHA-256 was `340a88b47b07ded2eda4266e2c6b011a6798fdc9c6a24e4978bb00f9d3a3e56d` and it passed with shell exit 0; exact stdout is `tests/evidence/oolite-oxz-real-assets-repro-guest.log`. This is the current source qualification, superseding the earlier standalone build fingerprint.
