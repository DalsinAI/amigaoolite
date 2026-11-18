# Original Oolite native 68040 compiler gate: 41 ObjC modules including JS

18 November 2026, 15:25–15:26 UTC. The dedicated AC090 guest previously displayed original Oolite splash artwork in a native GFOpenGPU SDL3 window for 240 frames (RC0). That is a standalone branded preview, NOT the actual Oolite engine. No original game title or gameplay has yet launched.

Using pinned original OoliteProject/oolite `21978c37b2f11c8664adf3e2062efc1d0c875979`, documented GFOS source overlays, real GNUstep Foundation and target SpiderMonkey 1.8.5 headers, GCC16.2 -O2 -m68040 -m68881 -noixemul, canonical GFOpenGPU SDL3/OpenAL/SGI GLU, `make oolite-core-fortyone-objects` successfully compiles 41 genuine Oolite Objective-C translation units into native objects. This is an object-code compilation gate, not a full engine link.

The latest twelve original Oolite `src/Core/Scripting/` modules compiled with no fatal errors:

- `OOJSGlobal.m`, `OOJSOolite.m`, `OOJSShip.m`, `OOJSPlayerShip.m`, `OOJSPlayer.m`, `OOJSScript.m`
- `OOJSFrameCallbacks.m`, `OOJSEntity.m`, `OOJSSystem.m`, `OOJSMission.m`, `OOJSMissionVariables.m`, `OOJSCall.m`

Previous 29 modules include original Universe with opt-in frame hook, original MyOpenGLView/SDL main and GameController, ShipEntity, PlayerEntity, HUD, GuiDisplayGen, OG OpenGL and GLU, ResourceManager, materials, sound and controller. All object files are genuine cross-compiled Oolite, not mocked classes.

**Crucial distinction:** the original OOJS* modules compile against generated SpiderMonkey 1.8.5 headers. This does **not** show that their external JSAPI functions link or work with the separate canonical GFOS `javascript.library` bridge. That remains a large functional integration task: verify actual engine linkage and one real original mission/script execution without stubs or false compatibility claims. The previous GFOS JS bridge tests (native property/ship routing, lifecycle and multi-task IPC) do not replace full original OOJS* integration.

No completed Oolite engine executable, original title screen, full UI or gameplay. Next: build complete original source graph, resolve actual runtime/symbol closure, connect Oolite startup/Universe and GFOS libraries and run its own title UI in guest. No timeline claim until full link diagnostics.
