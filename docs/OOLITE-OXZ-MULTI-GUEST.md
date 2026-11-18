# Oolite multi-file OXZ extraction + nested rollback (68040 guest)

18 November 2026. Pinned OoliteProject/oolite baseline `21978c37b2f11c8664adf3e2062efc1d0c875979`.

`tests/oolite_oxz_multi_gate.m` drives the GFOS extraction backend already wired through original Oolite `OOOXZManager` patch 0008. Two deliberately controlled fixtures exercise the complete read/decompress/write/rollback pipeline: `tests/fixtures/oolite-oxz-multi-valid.oxz` contains three entries (`manifest.plist`, `Config/shipdata.plist`, `Models/test.dat`); `tests/fixtures/oolite-oxz-multi-crc-bad.oxz` changes the stored model content without updating its CRC, so extraction fails *after* the manifest and config were written. Both fixtures must be staged as `SYS:Oolite/multi-valid.oxz` and `SYS:Oolite/multi-crc-bad.oxz` in the isolated guest. These are not real published Oolite expansion packs.

Native AmigaChrome AC090 68040 guest, GCC 16.2, original upstream MiniZip + zlib, GFOpenGNUstep Foundation 1.31.1:

```
OOLITE_OXZ_MULTI_BEGIN
OOLITE_OXZ_MULTI extracted=1 manifest=54 config=38 model=12 corrupt_rejected=1 rollback_clean=1 status=PASS
OOLITE_OXZ_MULTI_RC 0
OOLITE_OXZ_MULTI_END
```

Actual extracted files are present under the isolated `SYS:Oolite/AddOns/GFOSMultiProbe.oxp`, and the corrupt package left neither its destination nor a `.partial` directory. The native test binary SHA-256 was `bd9da1948a4e26d26804fd451f85dbf4bf052048bfa9e0b1b529419925f29693`.

The whole upstream OOOXZManager class and full game remain unlinked. A focused attempt to compile `OOOXZManager.m` against the complete pinned upstream tree reached game-engine headers requiring `SDL3/SDL_opengl.h` (covered by existing OpenGPU geometry patch 0001), then C99 `tgmath.h` long-double complex symbols (`csinl`/`ccosl`, unsupported in current cross-target libm), then `jsapi.h` from GuiDisplayGen/SpiderMonkey. No fake headers were used. The experimental `OOMaths.h` math-only GFOS variant is not yet published as a validated engine fix.

Remaining gates include actual upstream manifest/schema processing, installed expansion lifecycle, OXZ GUI/downloader, real OXZ compatibility, filesystem symlink races and complete ResourceManager game startup. Do not enable unattended arbitrary OXZ installation yet.
