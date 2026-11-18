# Oolite's real property-list loader on AmigaOS / GFOpenGNUstep

**18 November 2026**. Pinned upstream `OoliteProject/oolite` commit `21978c37b2f11c8664adf3e2062efc1d0c875979`. Actual native AmigaChrome AC090 68040/FPU guest, Kickstart 47.2 / Workbench 47.100. Source `src/Core/OOPListParsing.m` is ported via **patch** `patches/0002-gfos-native-plain-plist.patch` (not a replacement parser). Its original public APIs `OODictionaryFromFile`, `OOArrayFromFile`, `OOPropertyListFromData` and type validation remain in use. Cross-built using the same GCC 16.2 and GFOpenGNUstep Foundation 1.31.1 native archives as RC1.1.

## Changes made to Oolite's source

- `OOLITE_GFOS=1` switches on the AmigaDOS early-boot path. For **regular files** `CopyDataFromFile` uses GNUstep `NSData initWithContentsOfFile:` instead of `NSData oo_dataWithOXZFile:` (the latter requires the not-yet-ported OXZ virtual-file archive layer). OXZ paths must not be silently treated as supported.
- Deferred startup-only dependencies on Universe, OOLogging, OOStringParsing, OXZ categories. Oolite's own format parsing and array/dictionary type checks remain intact.
- The type-mismatch diagnostic in early native GFOS mode uses `fprintf(stderr, ...)` rather than Foundation `NSLog` printing Objective-C Class objects: the latter led to a test hang after successfully parsing the resources. The unmodified Oolite code path is preserved for other platforms. This temporary logging compromise requires future integration with the proper GFOS log sink.
- XML DTD rewrite is not used in GFOS early mode; OpenStep-style assets were verified. XML/OXZ remain open.

## Guest result: 7/7, exit 0

```
OOLITE_UPSTREAM_PARSER_RETEST_BEGIN
OOLITE_UPSTREAM_PARSER ship=194 planet=2050 trade=17 demo=27 empty=0
GFOS Oolite plist type mismatch
OOLITE_UPSTREAM_PARSER_PASS checks=7 failed=0
OOLITE_UPSTREAM_PARSER_RETEST_RC 0
OOLITE_UPSTREAM_PARSER_RETEST_END
```

- `OODictionaryFromFile` on **real Oolite files:** shipdata (194), planetinfo (2050), trade-goods (17).
- `OOArrayFromFile` on demo ships (27), empty pirate victim roles (0).
- Wrong type (`OOArrayFromFile` on a dictionary) returns nil, with expected diagnostic; missing path returns nil.
- The initial attempt hung on the wrong-type Foundation-based logging path; the successful retest (unmodified parser logic, modified log backend) ran without a hang and with shell RC 0. Earlier hung runner evidence is retained locally in isolated test lab; it is not called a pass.

Build source `tests/oolite_upstream_parser_test.m` using the real patched OOPListParsing.m from the pinned source and the `upstream-plist` Makefile target. Guest evidence `tests/evidence/oolite-upstream-parser-guest.log`. Guest tested binary SHA256 `8a658e049d9ad1d34a04edb4b66a5bc0731b21ee4638a19022c8617d6cfc0ddf`.

**Not yet a full ResourceManager launch:** Oolite's ResourceManager.m still depends on graphics, sound, JS/OXZ, universe, UI and caches. No full game executable, rendering or multicore game job is claimed. Next: real ResourceManager path subset, GFOS logging/lifecycle, then graphics and JS startup.
