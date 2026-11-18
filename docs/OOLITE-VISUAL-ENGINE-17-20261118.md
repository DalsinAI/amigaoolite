# Oolite original visual/gameplay engine: 17 native 68040 object compile checkpoint

18 November 2026, after successful native Oolite splash preview. Source pin: OoliteProject/oolite `21978c37b2f11c8664adf3e2062efc1d0c875979`, with documented GFOS Oolite source overlays. Build: GCC 16.2 native AmigaOS 68040/FPU, canonical GFOpenGPU SDL3/OpenAL/GLU, GNUstep, target SpiderMonkey 1.8.5 generated headers. No local git commits.

`make oolite-core-seventeen-objects` extends the previously qualified `oolite-core-ten-objects` target with seven **original** application/scene modules, all native ObjC `-O2` compilations (no temporary replacement classes):

| Original source | Purpose | GCC16 m68k result |
| --- | --- | --- |
| `src/Core/Entities/ShipEntity.m` | Original game ships and simulation behaviour | PASS |
| `src/Core/Entities/PlayerEntity.m` | Original player ship and state | PASS |
| `src/Core/GuiDisplayGen.m` | Original in-game GUI and display list | PASS |
| `src/Core/OOOpenGLExtensionManager.m` | GL extension detection | PASS |
| `src/Core/OOOpenGLStateManager.m` | GL state management | PASS |
| `src/SDL/MyOpenGLView+Input.m` | SDL window/input layer | PASS |
| `src/SDL/GameController+SDLFullScreen.m` | Original fullscreen controller | PASS |

All seven compiled successfully in direct and reproducible Makefile builds, and original `ShipEntity` methods are present in the actual resulting m68k object. Native original Oolite `Universe`, GL view, controller, startup `main`, ResourceManager, ship registry, sound, CPU, logging and GL helper were already qualified, hence **17 original source files** compile.

Independent visual preview evidence: `make oolite-native-splash` builds an SDL3 68040 program that displays the actual pinned Oolite `Resources/Images/splash.bmp` in a native 640x480 window. Native AC090 guest load 768x768 bitmap, rendered 240 frames, RC0 (`tests/evidence/oolite-native-splash-guest.log`). It is **not** linked to original `GameController` or `Universe`; no original title screen or gameplay has run. A framebuffer screenshot was not captured.

Why not playable yet: upstream Core/SDL contains roughly 247 `.m/.c/.cc` source files, including other Entities, Materials, Scripting, OOAL* audio, parser helpers and assets. We have not yet compiled the complete dependency graph or linked the final app; GNUstep, SpiderMonkey compatibility and GL runtime startup require full integration and testing. The earlier exploratory relocatable link (`ld -r`) of the original Universe and game frame hook failed and remains to diagnose.

Next priority: systematically compile the remaining source graph (preserving original code), resolve any missing GL/ObjC/JS symbols, link complete Oolite executable and start its original menu under AC090. No invented title-screen screenshot or timing estimate.
