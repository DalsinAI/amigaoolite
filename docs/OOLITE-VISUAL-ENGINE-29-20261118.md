# Original Oolite title/gameplay path — 29 native 68040 Objective-C objects

18 November 2026, 15:24 UTC. The actual Oolite-branded native GFOpenGPU SDL3 splash preview has already run in the dedicated AC090/AmigaOS 3.2.3 guest, rendering its original 768x768 splash artwork for 240 frames (RC0), documented in `docs/OOLITE-FIRST-VIEWABLE-20261118.md`. That is a standalone visual preview, NOT original Oolite gameplay or title UI.

`make oolite-core-twentynine-objects` now cross-compiles 29 genuine original Oolite source files at GCC16.2 `-O2 -m68040 -m68881 -noixemul`, using canonical SDL3 3.4.14, OpenAL (AHI), SGI GLU on GFOpenGPU, GNUstep Foundation and generated m68k SpiderMonkey 1.8.5 includes. The build retains pinned GFOS source patches and does not mutate/commit the upstream source tree.

The latest twelve successful module additions (all native m68k object compiler RC0) are:

- `HeadUpDisplay.m` — original in-game HUD
- `Entities/Entity.m`, `Entities/OOEntityWithDrawable.m`, `Entities/StationEntity.m` — foundational world entities/stations
- `Materials/OOMaterial.m`, `Materials/OOTexture.m`, `Materials/OOShaderProgram.m` — material/texture/shader ownership
- `OOOpenGLMatrixManager.m` — original GL matrices
- `OOOpenALController.m`, `OOMusicController.m` — original sound controller through GFOpenGPU OpenAL
- `SDL/OOSDLJoystickManager.m` — original input layer
- `OOCacheManager.m` — original cache

Previously the gate compiled: `Universe`, original `MyOpenGLView`, `GameController`, SDL `main`, resource manager, ship registry, sound source, native GFOS CPU and log, `OOOpenGL`, original `ShipEntity`, `PlayerEntity`, GUI display, GL extension and state managers, SDL input and fullscreen categories. Together these are **29 original Oolite modules**, not a complete game build.

Symbol inventory (from these 29 m68k objects using canonical m68k-amigaos-nm): 8,176 distinct defined symbols, 574 distinct unresolved external names across objects, of which 513 are not defined by this partial 29-object set. **These 513 are not necessarily defects**: many refer to remaining original Oolite modules or legitimate external GNUstep/SDL3/GL/SpiderMonkey libs. Examples of original Oolite dependencies remaining in the partial set include `OOExpandDescriptionString`, `OOJSFrameCallbacksInvoke`, `OODeepCopy`, plus external GL calls and `OOGFOSGameFrameUpdate` (implemented in GFOS bridge). Only a complete linked application can distinguish actual missing implementations from normal library references.

The pinned Core/SDL source tree contains around 247 `.m/.c/.cc` source files, before conditional Meson selections; do not interpret 29/247 as a percentage of port completion. **Next requirement:** build the full original source closure from Meson source lists, include GFOS patch/headers, qualify binary link with the original startup, and test title UI in native AC090 guest. Previous exploratory partial `ld -r` link failed, not ignored.
