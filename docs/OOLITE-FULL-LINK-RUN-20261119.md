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
- To catch it next time, do one of these:
  1. Have the trace write to stdout (or to both stdout and stderr) so `Run >file` captures it.
  2. Have the trace append to a fixed file, such as `T:gfosjsapi-trace.txt`.
- Once the trace is in the file, the first `gfosjsapi: pending` line before "Found Scripts/oolite-global-prefix.js" shows which setup call raised it (likely a define or class-init glue call during `JS_InitStandardClasses` or Oolite's `InitOOJS*`).

## Still to check

- Top-level `var` and `function` declarations in a script run against an object. Under `with (this)` plus `eval`, they land in `execIn`'s own scope, not on the object; SpiderMonkey puts them on the object. Oolite's world scripts set `this.name`, `this.startUp = function ...` on `this`, which works. Plain `var x` or `function f()` at the top of a script would stay local. A possible fix: evaluate in a fresh function whose scope chain is the object, then copy the declared names across (the gfosjsapi gate can test both forms).
- How run4 ended: its oolite process (9) was still alive and ignored Ctrl-C; the screen showed the blanker. Restart the lab before run5.

## Next steps

1. Push the execIn commit. The repo moved to amigagfosports, and Claude's GitHub app has no access to that org yet (push 403).
2. Add `amigagfosports/amigaoolite` to Cradle's stove_build repo list (amicradle repo list); it is not on it yet.
3. Rebuild `tools/cradle-fulllink`, lab_put to `SYS:OoliteT5/oolite`, restart the lab, then `SetEnv GFOSJSAPI_TRACE 1`, `Run >SYS:OoliteT5/run5.out oolite -nosplash` and take a screenshot.
4. Go on until the title screen, then gameplay.

Team 5
