# Oolite controller integration checkpoint — 18 November 2026

This review compares pinned OoliteProject/oolite `21978c37b2f11c8664adf3e2062efc1d0c875979`, canonical DalsinAI/gfopensdk `4ac683894edf8c0e30e36d9318662fdde0ed39c9`, and GFOpenRTG source `6e3ae8e3abb98de4af01c577ed2663866d2fedc1` on daletop.

## Reuse decision

Preserve Oolite's Objective-C `GameController` and `OOSDLJoystickManager` for application/game state and event routing. Reuse GFOpenGPU SDL3 for window/event/render integration and GFOpenInput for physical controller discovery, mapping, state, hot-plug and optional exclusive control. Do not route Oolite through ACGame's indexed/HAM8 platform lifecycle: it would create competing screen/input ownership.

## Verified source details

- GFOpenSDK `Include/libraries/gfopeninput.h`: frozen v1.2 API; standard 21 buttons, six axes, device IDs, GetControllerInfo, OpenController, ReadState, raw input, notifications and controller mapping; lowlevel/CD32 fallback supported.
- GFOpenRTG SDL2 `library/modules/sdl2/src/joystick/amigaos3/SDL_sysjoystick.c`: full GFOpenInput integration already exists and may inform SDL3 port, but it uses SDL2 internal APIs and must not be pasted into SDL3.
- Current GFOpenRTG SDL3 `library/modules/sdl3` joystick backend (reconstructed SDL3 3.4.14 source): uses `lowlevel.library` `ReadJoyPort` directly, exposes two digital axes and 2/7 buttons, and returns false from `OS3_JoystickGetGamepadMapping`. This does not establish modern GFOpenInput pad support.
- Oolite `src/SDL/MyOpenGLView+Input.m` handles SDL3 joystick/gamepad events; `src/SDL/OOSDLJoystickManager.m` opens enumerated SDL3 joysticks. Its `dealloc` did not explicitly close these handles; patch `patches/oolite/0014-gfos-sdl3-joystick-lifecycle.patch` now closes opened handles and removes an obsolete bounds check.

## Evidence and limits

Patch 0014 dry-ran clean against the pristine pinned Oolite checkout and the resulting modified source compiled to a real native GCC16.2 68040 Objective-C object on daletop (15,352 bytes); SHA-256 of the GitHub patch matched the tested patch. No live controller UAT, full original engine relink, or new title-screen/gameplay qualification was performed by this change.

## Next gates

1. Port SDL2 GFOpenInput device discovery/notification/mapping logic to SDL3's joystick driver with SDL3-native internals, keeping `ReadJoyPort` as an explicit fallback. Validate device-ID stability, disconnect and reconnect, analogue axes and triggers, the shared/exclusive policy and correct pad mapping.
2. Build `SDL3.module` with the new backend; test no-device, CD32/lowlevel, synthetic GFOpenInput pad, removable pad, and app shutdown on isolated AC090 guest.
3. Apply Oolite patch 0014 to isolated pinned source during the full-build recipe and test repeated manager creation/destruction. Keep regression evidence distinct from a game-UAT pass.

No standalone alternative game controller or JavaScript runtime should be added.
