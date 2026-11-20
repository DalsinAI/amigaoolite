# The GFOS runtime under Oolite: script task and Foundation (19 Nov 2026)

Team 5. Lab `t5-oolite-20261119` (Instance-32 copy + javascript.library 2.0 / JavaScriptEngine / json.library from Team 1's lab). Route decided by Dale on 19 Nov:
- no SpiderMonkey code or library;
- Oolite keeps SpiderMonkey's API *names* through a header implemented on javascript.library.

## 1. The game runs on javascript.library's script task

A JSTask's VM runs on its own thread. A library call from another task is handed over and waited for; a call from the script task itself is a plain C call. So `main()` keeps only process set-up and posts the whole game to the script task (`JS_Post(task, gameMain, data, TRUE)`). There, every JS call in either direction is direct, and host functions may use Objective-C and the C library: the launcher is parked, so nothing runs concurrently.

**The one trick:** libnix pthreads answer `pthread_self() == 0` (the main thread) for any task they don't know. So the game job takes over pthread slot 0 for the script task and gives it back afterwards. Without this, GNUstep shares the main thread's state from another task and deadlocks. Requires `libpthread.a` with `__threads` made global (`objcopy --globalize-symbol=__threads`; slot 0's Task pointer is at offset 20). See `tests/oolite_js_game_on_script_task_gate.m`.

Gate on AC090: **PASS, 0 failures** (`tests/evidence/oolite-game-on-script-task-gate-ac090-20261119.log`).

| Measured on AC090 (lab, quiet) | per call |
| --- | ---: |
| one handover (empty `JS_Post`, wait) | 12 µs |
| C→JS call from the launching task (5 handovers) | 211 µs |
| C→JS call on the script task (NewLong + Call + ToLong + 2 Free) | 63 µs |
| JS→C call, empty native (incl. the JS loop) | 51 µs |
| JS→C call doing NSString work | 75 µs |

On the script task the handover cost disappears. What's left, about 50–60 µs a crossing, is JavaScriptCore's C-API path on the emulated 68040. Against the 5 ms per-frame budget that is roughly 80–100 crossings a frame. Counting real crossings per frame in upstream Oolite is the next measurement. Cheaper host calls belong in javascript.library.

## 2. Faults fixed under it (all block Oolite whatever the JS route)

| # | Fault | Symptom | Fix |
| --- | --- | --- | --- |
| 1 | `-fobjc-exceptions` without `-fexceptions` at **compile** | every `@throw` aborts | compile with `-fobjc-exceptions -fexceptions -D_NATIVE_OBJC_EXCEPTIONS` |
| 2 | no `-fexceptions` at **link** | the program never registers its exception frames; every throw aborts, even with 0 extra objects (`gfos_objc_eh_probe`: abort without, PASS with) | link with `-fexceptions` |
| 3 | GNUstep Base built for 68000 soft-float | every double into/out of Foundation is garbage (`timeIntervalSinceNow` = 2×10^58) | rebuild for `-m68040 -m68881` |
| 4 | GNUstep Base built with setjmp exceptions (`BASE_NATIVE_OBJC_EXCEPTIONS 0`) | Foundation's exceptions are invisible to `@catch` | native exceptions (`patches/gnustep/0003`) |
| 5 | libnix `getpwuid`/`getgrgid` ask the TCP/IP user database (libsocket), which GFOS doesn't have | no user, no home; NSUserDefaults hung, also after each write (group lookup) | `src/Core/OOGFOSPasswd.c`: one user ($USER or `amiga`), home $HOME or PROGDIR:, one group |
| 6 | libnix reports uid 0 | GNUstep chowns every file and directory it makes, which fails; `createDirectoryAtPath:` returned NO | the shim reports uid/gid 1 |
| 7 | GNUstep has no Amiga path model | `SYS:x` isn't absolute; it can't find its own executable; asserts inside `+[NSUserDefaults initialize]`, whose NSLog then deadlocks | `patches/gnustep/0002`: `Name:` is a root; `SYS:` + `x` = `SYS:x` |
| 8 | bare `argv[0]` | same assertion | `OOGFOSAbsoluteArgv()` before `GSInitializeProcess()` |

Defaults probe on AC090: NSUserDefaults read/write/synchronize/unlock, NSTimeZone, NSLog, `@throw` and `+raise:format:` caught, all **PASS** (`tests/evidence/gfos-foundation-defaults-probe-ac090-20261119.log`). Defaults live in `ENVARC:GNUstep/Defaults/<program>.plist`.

Checked and *not* faults: AC090's `TST.L An`, libnix's callee-saved registers, `struct stat` layout, `-fileExistsAtPath:isDirectory:`.

## 3. Build

`make -f Makefile -f build/gfos-runtime.mk gfos-runtime-gates`:
- `tools/gfos-foundation/build-gnustep-base-gfos.sh` rebuilds Base from the existing configured tree plus `patches/gnustep/0002–0003` (same 240 members, about a minute);
- then the gate and probes, with the flags and link rules above.

The Foundation rebuild belongs in GFOpenGNUstep and the SDK (Dale, 21:17: release candidate and SDK update). Until then, this script is how amigaoolite gets it. Still pointing at `/tmp` inputs like the rest of this repo; moving to the stove (open8) is separate.

## 4. Next
1. The SpiderMonkey-API header on javascript.library (`src/Core/gfosjsapi/`), then the first full link with no SpiderMonkey.
2. Count crossings per frame in upstream Oolite.
3. Apply rules 1–2 and the new Foundation to `build/full-link.mk`.
