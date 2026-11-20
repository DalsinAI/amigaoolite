# Oolite full link: lab runs, 19 Nov 2026 (Team 5)

Branch `team5/oolite-full-link-20261119`, stacked on `team5/gfosjsapi-header-20261119` (PR #5).
Built with `stove_build` on `tools/cradle-fulllink`. Lab `t5-oolite-20261119`, game in `SYS:OoliteT5`.

## Run 4 (binary at 8920417)

Working:
- GNUstep/Foundation startup, settings, AddOns search paths.
- SDL3 with OpenGL (DalsinAI/gfopenrtg#274): 1280x720 window, OpenGL 4.3 Compatibility, Mesa 26.2.4, virgl on an AMD RX 460, 284 extensions.
- JavaScript context, shaders, joystick, AIs, Models and Textures found and loaded.

Stopped at:
- `oolite-global-prefix.js`: `ReferenceError: Can't find variable: special`.
  Cause: gfosjsapi's `execIn` ran `eval` without the target object in scope, so a script run against an object could not name its properties.
- Before it, Oolite's `script.javaScript.load.waitingException`: "Prior to loading script ... oolite-global-prefix.js, there was a pending JavaScript exception". Something during JS engine setup leaves an exception pending.

## Fix: scripts run against an object see its properties

`src/Core/gfosjsapi/gfosjsapi.c`, the glue:

```
"    execIn: function(__gfosjsapiSource) { with (this) { return eval(__gfosjsapiSource); } }\n"
```

`JS_ExecuteScript` already calls `execIn` with the target object as `this`, so this one line is the whole fix.
Commit: "gfosjsapi: scripts run against an object see its properties".

## The pending exception before the first script

- Every exception gfosjsapi makes pending goes through `setPendingOwned`, and with `SetEnv GFOSJSAPI_TRACE 1` it prints `gfosjsapi: pending exception: <text>`. `JS_SetPendingException`, `JS_RestoreExceptionState` and failed glue calls all go that way.
- run4.out has no `gfosjsapi: pending` line, although run4 had the trace on. The trace goes to **stderr**. `Run >file` captures stdout only, and on this Shell (Kickstart 47.2) `*>file` is not accepted, so stderr went to the console instead of the file.
- Fixed for run5: the trace now goes to **stdout**, numbered, with the exception's JS stack (`gfosjsapi: pending exception N: ...` and `... N stack: ...`), so `Run >SYS:OoliteT5/run5.out` catches it. The first one before "Found Scripts/oolite-global-prefix.js" is the setup call that raised it; the stack names the glue helper (defineValue, defineAccessor, freeze, ...).
- Oolite clears that exception and carries on, so it is not what stopped run4; `special` was.

## Gate

`tests/gfosjsapi_gate.c` now checks the prefix script's pattern: a strict script run against an object sees the object's `special` as a variable, still sees the globals, and `this.name` lands on the object.

## Still to check

- Top-level `var` and `function` declarations in a script run against an object stay in `execIn`'s own scope; SpiderMonkey puts them on the object. Checked in Node with the same glue: the script's own functions still reach them (closures work), so it only matters when something outside the script looks them up as properties. oolite-global-prefix.js has none (strict, everything through `this` and an inner function). Left as it is until a script needs it.
- How run4 ended: the game window ("Oolite v1.93.1-255 by AmiCradle Team") opened at 1280x720 and stayed black and not redrawing; run4.out stopped at 23:27:59, about 20 s after the start, and oolite (process 9) ignores Ctrl-C. The first 50 KB of the log ends in the texture list; the agent and lab_get cut the rest, and the lab has no grep or tail, so where it hangs is unknown. Run5, with the prefix fix, is the test. Restart the lab before it.
- The lab's blanker is now off at boot (S:User-Startup, backup S:User-Startup.before-noblanker).

## Next steps

1. Push the execIn commit. The repo moved to amigagfosports, and Claude's GitHub app has no access to that org yet (push 403).
2. Add `amigagfosports/amigaoolite` to Cradle's stove_build repo list (amicradle repo list); it is not on it yet.
3. Rebuild `tools/cradle-fulllink`, lab_put to `SYS:OoliteT5/oolite`, restart the lab, then `SetEnv GFOSJSAPI_TRACE 1`, `Run >SYS:OoliteT5/run5.out oolite -nosplash` and take a screenshot.
4. Go on until the title screen, then gameplay.

Team 5

## 20 Nov 2026: runs 5 to 12 (branch team5/oolite-define-twice-20261120)

Built with Cradle's stove_build under the old name DalsinAI/amigaoolite (GitHub redirects it), job by job, into lab t5-oolite-20261119.

| Run | Stopped at | Fix |
|---|---|---|
| 5 | prefix: `Can't find variable: Ship` | Upstream's `sShipProperties` lists `homeSystem` twice; JavaScriptCore refused the second permanent definition, so JS_InitClass never defined `Ship` (and that was the exception left pending before the first script). The glue keeps the first definition, as SpiderMonkey does. |
| 6 | prefix after the JavaScript reset: `Attempting to change the getter of an unconfigurable property` | JS_ClearScope did nothing, and JavaScriptCore's global outlives the reset. What is defined on the global is kept configurable and JS_ClearScope deletes it, leaving the built-ins. |
| 7, 8 | after "Running world scripts", no JavaScript running | GFOSJSAPI_TRACE 2 (every C/JS call to T:gfosjsapi-trace.txt) showed the hang was outside JavaScript. |
| 9 | in startAnimationTimer (patch 0018 markers) | GNUstep's NSRunLoop never returns on the script task. Patch 0019: the game loop calls performGameTick: itself. |
| 10, 11 | first frame, drawing the player's ship (patch 0020 markers) | The mesh waited in ensureFinishedLoading for textures from the async work threads, which never deliver. Patch 0021: async work runs on the game task. |
| 12 | **runs**: "Loading complete in about 20 s", over 1,500 ticks, the title screen's demo ships drawn textured and shaded with OpenGL 4.3 (virgl), turning and changing | |

| 13 | **plays**: title screen with text and menu; Start New Commander, Normal Start; docked at Lave (status screen, full HUD); F1 launches into flight, with Lave's generated planet and atmosphere on the forward view | Patch 0022: GNUstep converts the font's CP1252 through iconv, which the GFOS Foundation lacks, so every string came back nil and drew nothing. Built-in Latin-1/CP1252 conversion when it does. |

After launch (run 13), the game stopped: the lab's 0.62 GB of Amiga RAM was full (0.6 GB in use, per the Workbench title bar), and the GL bridge gave "out of memory" (0x505) drawing the Coriolis station and the Cobra III, whose textures go up to 2048x2048 normal maps.

Still open, from run 13:
- Memory in flight: more RAM for the lab, or smaller textures and less detail on GFOS.
- Lave's planet texture takes about two minutes to generate after Normal Start (OOPlanetTextureGenerator, now inline on the 68040).
- The theme music: OpenAL streaming logs "Incomplete, trying next for OoliteTheme.ogg" every frame.
- The trace patches 0018 and 0020 (launch and first-frame markers) can come out once the game reaches gameplay.

Team 5
