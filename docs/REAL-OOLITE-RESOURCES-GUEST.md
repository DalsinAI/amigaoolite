# Actual Oolite resources parsed by GFOpenGNUstep in AmigaChrome

**18 November 2026**. Source files retrieved unchanged from `OoliteProject/oolite` pinned commit `21978c37b2f11c8664adf3e2062efc1d0c875979`, paths `Resources/Config/*.plist`. The game data is **not invented** and is not vendored in this repo. Tested on isolated AmigaChrome AC090 native 68040/FPU, Kickstart 47.2 / Workbench 47.100, using GNUstep Base 1.31.1 and GFOS static Foundation bridge.

## Real guest stdout and result

The test uses native `NSFileManager fileExistsAtPath:`, `NSData dataWithContentsOfFile:` and `NSPropertyListSerialization propertyListFromData:mutabilityOption:format:errorDescription:`. It verifies successful OpenStep (format 1) parsing, expected Objective-C array/dictionary types and reference entry counts, including Oolite's **intentionally empty/deprecated** pirate-victim-roles list. The guest's AmigaDOS paths are `SYS:Oolite/Config/<filename>` on the mounted System volume.

| Source file | Bytes | Entries | Result |
| --- | ---: | ---: | --- |
| demoships.plist | 308 | 27 | PASS |
| global-settings.plist | 1,318 | 3 | PASS |
| pirate-victim-roles.plist | 466 | 0 | PASS, intentionally empty deprecated array |
| role-categories.plist | 4,199 | 16 | PASS |
| trade-goods.plist | 8,321 | 17 | PASS |
| shipdata.plist | 128,966 | 194 | PASS |
| descriptions.plist | 110,079 | 1,186 | PASS |
| planetinfo.plist | 2,713,698 | 2,050 | PASS |

**Actual guest:** `OOLITE_REAL_ASSETS_PASS files=8 failed=0`, shell `OOLITE_REAL_8ASSETS_RC 0`; raw `tests/evidence/oolite-8-real-assets-guest.log`. `planetinfo.plist` takes longer than the smaller data files, but finished normally. This is real UTF-8/OpenStep parsing (with GNUstep's historic parser), not a placeholder, host-side parse or synthetic array.

## Reproduction and scope

The `real-assets` Makefile target builds `tests/oolite_actual_plist_probe.m` as a native AmigaOS loadseg binary using the same GCC16, GFOpenGNUstep Base, libobjc and libffi as the working RC1.1. It reproducibly produced SHA256 `04a2f277f32b6762a701283582d5a9e9693b6b471d29b39e143345fe4e0569a9`, matching the actual guest-tested binary. The original OpenStep `.plist` files must be downloaded from the pinned upstream commit and staged at `SYS:Oolite/Config/` **on a verified mounted guest volume** before running `SYS:ACAgent/oolite-real-assets-68040`. Do not assume a host-side DH1 folder mounts DH1: (see `RESOURCE-FILESYSTEM-GUEST.md`).

## What's not yet working

This tests **GFOS Foundation directly**, not the full upstream `ResourceManager.m`, which imports Oolite UI, graphics, JS, sound, manifest, OXZ and caching subsystems and cannot yet be linked in isolation. File parsers are an important *startup dependency*, but **the Oolite executable does not yet boot or render**. Next priorities are actual upstream ResourceManager dependency reduction and live resource enumeration, then rendering via GFOpenGPU, input/sound via GFOpenUp, SpiderMonkey and GFOS `NSRunLoop`. Team 4's multicore SDK work remains untouched; compute jobs will use their canonical `gfopenmulticore.library` after SDK publication and real board testing.
