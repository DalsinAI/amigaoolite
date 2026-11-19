# Oolite GFOS RC0 engineering checkpoint — 19 November 2026

**Status:** recovery/build-engineering prerelease, **NOT a playable Oolite RC**. This checkpoint exists to prevent future rebuilds from restarting at source discovery. GitHub is the durable source of truth; binary and guest-data snapshots are attached to the GitHub prerelease, not committed into git history.

## Verified immutable source points

- `DalsinAI/amigaoolite` main (before this document): `1fb51390179dd07f572baa1d76c0c27edf1450db`.
- Pinned upstream `OoliteProject/oolite`: `21978c37b2f11c8664adf3e2062efc1d0c875979` (242 original source translation units, historically compiled and linked on Nov 18).
- `DalsinAI/gfopengnustep`: `8ba0a7f94dc27230a1e60be36793fcc9593f72f0` (patch 0005; reproducible `native-defaults-68040` gate; native guest save/reload proof).
- `DalsinAI/gfopenrtg`: `cec6e818105b8e9ee80cf7b1433a1e42b4017834` (SDL3 GFOpenInput backend; guest input and rumble proof).
- Compiler: OpenAmigaGCC 16.2, 68040, AmigaOS 3.2.3 / AC090 guest.

## Included historical working materials

- Full native original Oolite link as three **historical** trace-stage executables: `Oolite-gfos-controllertrace-68040`, `Oolite-gfos-defaulttick-68040`, `Oolite-gfos-gameboot-68040`. All three source-binary checksum entries pass. `gameboot` is 14,717,972 bytes and its `-help` returned RC0 in an isolated guest. A normal start entered original `GameController`, reached `OOLITE_GFOS_LAUNCH before-view-init`, and stopped. No real title screen or gameplay is proven.
- The exact guest Oolite `Resources`, `Config`, `Scripts`, `addons`, `AddOns` directories as a separate resource snapshot; do not treat test fixtures under addons as a release gameplay test.
- Corrected GNUstep Base **private** archive built 19 November, after the historical full game executable, with patch 0005, plus separately compiled patched `MyOpenGLView.o` and the GFOS GCC16 tgmath declarations. These are build inputs; they are **not** inside the historical gameboot binary.
- Pinned source patches 0001–0015 in repository; `build/full-link.mk` already replays patch 0015. The separately compiled view object is qualified only at the compiler gate.

## Rebuild/next step (no reimplementation)

1. Checkout the tagged `DalsinAI/amigaoolite` source; use upstream Oolite at the SHA above. Get GFOpenRTG and GFOpenGNUstep at their exact SHAs above. Do **not** replace the canonical `javascript.library` architecture with legacy static JS for release.
2. Rebuild GNUstep Base using GFOpenGNUstep `patches/gnustep-base/0005-gfos-native-userdefaults-filesystem-bootstrap.patch` and `make native-defaults-68040`; retain original GNUstep archive separately.
3. Apply the existing original source patches in `build/full-link.mk`, including `0015` for the view startup. Reuse `build/gfos-amiga-tgmath-compat.h` for GCC16. Restored Linux-host Python 2.7 and old SpiderMonkey source are diagnostic-only historical dependencies. Native AmigaPython 2.7.18 is a separate project and must not be replaced.
4. **Missing for a fresh complete diagnostic relink in this checkpoint**: a qualified original SpiderMonkey 1.8.5 `libjs_static.a` in the recovered workspace, plus exact matched link path validation. The prior full link is proven, but a new post-patch full link and title/gameplay guest run have **not** occurred.
5. Re-link the original engine against the updated Foundation and patched view, run normal startup in a disposable AC090 guest with a bounded timeout, capture first view trace, confirm real SDL3 window/title/ship/world and clean exit. Then migrate original JSAPI usage to the canonical GFOS `javascript.library` and qualify normal gameplay before promoting to playable RC1.

## Explicit release gates

| Gate | Status |
|---|---|
| Original 242-source full engine historically linked | PASS |
| Native historical `-help` on AC090 | PASS |
| GFOpenInput/SDL3 native component probes | PASS |
| GNUstep patched defaults save/reload + Foundation core/date/IPC | PASS (stale-lock recovery remains open) |
| Patched `MyOpenGLView.m` compiled to native object | PASS |
| Newly relinked complete executable containing Foundation patch 0005 + view patch 0015 | **NOT VERIFIED** |
| Real Oolite title screen / flight / save/load | **NOT VERIFIED** |
| Full GFOS canonical `javascript.library` engine linkage | **NOT VERIFIED** |
| Playable RC1 release | **NO** |

No local git commits were created. Source changes are published using GitHub remote commits. The release tag is the immutable recovery anchor; generated assets have checksums in the release manifest.
