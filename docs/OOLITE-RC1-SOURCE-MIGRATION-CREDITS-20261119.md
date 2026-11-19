# RC1: second real original Oolite source replacement (98 APIs remain)

19 November 2026 | Team 1 | Original Oolite 1.93.1, GCC16.2 / 68040.

## Verified achievement

The complete original Objective-C Core/OOStringParsing.m has now been
compiled for AmigaOS 3.2.3, with only its optional
OOStringFromDeciCredits JavaScript formatting boundary ported to
canonical javascript.library v2 through OOGFOSCredits.

The native game still owns the uint64_t OOCreditsQuantity; JavaScript
receives the same decimal number solely for the optional OXP
formatCredits callback. No new JS game logic or SpiderMonkey shim.
The C bridge previously passed native AC090 tests, including large
64-bit credit values.

The genuine original Core_OOStringParsing_m.o is 26,604 bytes with SHA-256
4a610fe70a32777efad6a46533b8ddded30bd1297c1938cf08a2e4de30ce2851.
It has zero unresolved JS_* references.

Replacing only that original object in the 242-object corpus:
242 objects / 52 dependent / 99 unique historical JS_* APIs.
Then also replacing the separately qualified original
Core_OORegExpMatcher_m.o:
242 objects / 51 dependent / 98 unique historical JS_* APIs.
The regexp object SHA-256:
d714069147eaf76686273b5ee22db77c2365977ecaf1f51865c962d980d162f3.

Original object audit: unknown=0, overlap=0, no objects removed.
Evidence:
- tests/evidence/oolite-original-242-object-jsapi-98-20261119.json
- docs/OOLITE-JSAPI-LINK-AUDIT-98-20261119.md

## Authentic headers restored from pinned sources

No fabricated SDL3, GL, NSPR or JSAPI headers.

1. GFOpenRTG SDL3 UPSTREAM.json pins SDL3 3.4.14; official source
   archive SHA-256
   30d4aa2b3037718142b32dffd4e72f917ebb6cc5227150e7bb9c45efb2153aeb.
   Used authentic include subtree and GFOpenRTG 68k patch
   patches/sdl3/0001-amigaos3-platform.patch to correct the target
   2-byte pointer-alignment assertion, platform and video headers.
   Only headers were extracted, so source-file hunks were skipped.
2. OoliteProject/mozillajs-linux pinned source commit
   bc12f7124166c199925f3cea37d2732d3704213d,
   recovered for compile-time original legacy headers ONLY, not
   production JS execution. Generated m68k js-config.h using its
   cross-configure; generated big-endian jsautocfg.h with
   jscpucfg.cpp, CROSS_COMPILE and FORCE_BIG_ENDIAN.
   Old configure probes were adjusted in a disposable copy.
3. GFOpenRTG GLU upstream pins Mesa glu-9.0.3,
   SHA-256 bd43fe12f374b1192eb15fe20e45ff456b9bc26ab57f0eee919f96ca0f8a330f.
   Used authentic include/GL/glu.h.
4. Recovered src/Core/gfos-amiga-tgmath-compat.h byte-for-byte
   from checksum-qualified RC0 release build recovery archive.
   This declares missing GCC16 m68k tgmath functions; no fake
   implementations or linker symbols.

Build with make js-original-credits-object and matched
GFOS_SDL3_HEADERS, GFOS_OPENAL_HEADERS, GFOS_JS185_HEADERS,
GFOS_GLU_KIT, GFOS_JS_SDK, PREFIX and UPSTREAM, then run
jsapi-two-original-replacements-audit against original object corpus.

## RC1 status

NOT PLAYABLE. Remaining: 98 SpiderMonkey APIs in 51 original
object files, no GFOS-only complete Oolite link.
Next high-value migrations are OOJavaScriptEngine.m
(48 APIs, 25 unique) and OOJSScript.m (28 APIs, 12 unique).
Original engine must call OOGFOSCreditsBindTask at JSTask
initialization and UnbindTask before shutdown; source compile alone
does not wire this lifecycle. Experimental JS_Stop v3 remains
unqualified and must not ship in RC1.
