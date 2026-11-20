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
- How run4 ended: its oolite process (9) was still alive and ignored Ctrl-C; the screen showed the blanker. Restart the lab before run5.

## Next steps

1. Push the execIn commit. The repo moved to amigagfosports, and Claude's GitHub app has no access to that org yet (push 403).
2. Add `amigagfosports/amigaoolite` to Cradle's stove_build repo list (amicradle repo list); it is not on it yet.
3. Rebuild `tools/cradle-fulllink`, lab_put to `SYS:OoliteT5/oolite`, restart the lab, then `SetEnv GFOSJSAPI_TRACE 1`, `Run >SYS:OoliteT5/run5.out oolite -nosplash` and take a screenshot.
4. Go on until the title screen, then gameplay.

Team 5
