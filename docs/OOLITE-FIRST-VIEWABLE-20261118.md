# First visible Oolite-branded native window — AC090 guest

18 November 2026, 15:17–15:19 UTC. Verified actual GitHub branch main before work; pinned original Oolite HEAD `21978c37b2f11c8664adf3e2062efc1d0c875979`. Source: `tests/oolite_native_splash_view.c`. This is an **explicitly separate native SDL3 splash preview**, not a false claim that the Oolite game engine has linked or booted.

The original unmodified upstream asset `Resources/Images/splash.bmp` is a 768×768 24-bit bitmap (1,769,526 bytes), loaded from `SYS:Oolite/Resources/Images/splash.bmp` in a native 640×480 SDL3 window named `Oolite - AmigaOS Native Preview`. The code creates a real SDL3 window/renderer, loads BMP, creates GPU texture, renders an image-centered native display for up to 240 frames at 50ms, supports close/key exit, disposes GPU resources, and returns a nonzero exit code on init/window/load/texture/render failure. This is NOT a static canned screenshot or fake GUI.

The native GCC16 m68k 68040/FPU preview links to **GFOpenRTG canonical `libSDL3.a`**, which loads its real `SDL3.module` via GFOpenGPU. Built at `-std=gnu11 -O2 -m68040 -m68881 -noixemul -Wall -Wextra -Werror`, native 199 KiB launcher. The previously qualified dedicated AC090 AmigaOS 3.2.3 guest has the canonical SDL3.module installed in its own `LIBS:GFOpenGPU/`. The actual Oolite splash asset was copied into its `SYS:Oolite/Resources/Images/` folder. Runtime guest output:

```
OOLITE_NATIVE_VIEW_BEGIN
OOLITE_SPLASH SOURCE=SYS:Oolite/Resources/Images/splash.bmp WIDTH=768 HEIGHT=768
OOLITE_SPLASH status=PASS frames=240
OOLITE_NATIVE_VIEW_RC 0
OOLITE_NATIVE_VIEW_END
```

The executable is reproducible with `make oolite-native-splash GFOS_SDL3_HEADERS=<gfopen-sdl3-build/include> GFOS_SDL3_LIB_DIR=<gfopen-sdl3-build> PREFIX=<gcc16-stove> OUT=<output>`. No local git commits and no shared system install. Exact run log: `tests/evidence/oolite-native-splash-guest.log`.

**Limitations**: SDL3 reports window creation, texture load, rendering loop and clean exit; a pixel screenshot of the GUI was not captured in this run. The actual game world, shaders, OXPs, menus and JavaScript-driven game startup have NOT run. The engine's ten central ObjC files compile to object code separately, but original full source graph and link unresolved. The visible milestone is a branded **boot preview**, not a playable title screen.

**Next actual game milestone**: link the full original Oolite source graph, determine resolution of GL.module's symbols and the ObjC/JS framework dependencies, boot original SDL main and GameController under AC090, then capture a real framebuffer screenshot of Oolite's title UI. Do not use this preview as gameplay evidence.
