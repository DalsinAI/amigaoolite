# Original Oolite view startup — GFOS AC090 checkpoint (18 Nov 2026)

## Real guest observation

Original *full Oolite executable*, `Oolite-gfos-gameboot-68040`, 14,717,972 bytes, recovered from historical Oolite guest and run in an independently cloned AmigaOS 3.2.3 AC090 native 68040 lab on `/mnt/xs1000/oolite-sdl3-input-uat-20261118` (host 9241, native bridge 9240). `-help` returned 0 and displayed original Oolite version/help, preserved `evidence/oolite-gameboot-help-on-input-lab-pass.log`.

Normal invocation `Oolite-gfos-gameboot-68040 -nosound -noshaders` proceeded through `GameController -init` and all recorded post-Foundation/bootstrap stages, then reached `OOLITE_GFOS_LAUNCH before-view-init` with no subsequent trace or return. Guest stdout copied to `evidence/oolite-gameboot-before-view-init-stall-guest.log`; only this isolated clone's native bridge was terminated afterward; historical guest and other teams' instances unchanged. A running/unfinished command is a **failure/stall, not a PASS**.

## Likely root cause, not yet proven by a complete new guest run

Pinned Oolite `src/SDL/MyOpenGLView.m`, `-init` line ~315, immediately invokes `[NSUserDefaults standardUserDefaults]`, and later reads splash/v-sync/HDR and mouse-flight-sensitivity preferences. The historical separate native Foundation preferences probe independently printed `OOLITE_DEFAULTS_PROBE before-defaults` and stalled inside the same API. Prior original Oolite `GameController -init` also stalled on early `NSUserDefaults`; the successful source patch `0013-gfos-controller-default-tick.patch` bypassed that call on GFOS only and got startup into the view path.

**Patch 0015** applies an equivalent guarded *startup-only* GFOS bootstrap: use Oolite's defaults for splash, native v-sync off, 8-bit components and mouse sensitivity 0.95f until native GNUstep preferences are qualified. Non-GFOS code remains unchanged. It includes guarded console traces after major view initialization stages to locate any subsequent blockage. The patch replayed cleanly against pinned Oolite `21978c37b2f11c8664adf3e2062efc1d0c875979`, exactly reproducing the tested modified source; Makefile full-link object dry-run returned zero.

**NOT COMPILE/TESTED AS A FULL GAME:** native standalone compilation currently stops in the unprepared original source on `OOQuaternion` complex trig/compiler headers and unavailable pinned SpiderMonkey 1.8.5 `jsapi.h`; these are historic full-engine dependency/setup issues, not proof that patch 0015 fixes startup. The old full-link recipe still uses a standalone SpiderMonkey archive and Vorbis compatibility libs, and is not the approved production GFOS-only architecture. No successful title-screen or gameplay run should be inferred.

## Required next gate

Reconstitute the pinned 242-object build dependency environment or integrate original scripting directly with canonical `javascript.library`; apply all original source patches including 0014/0015 in isolated patched source, compile/link a new executable, run normal launch in a dedicated guest and require view stage traces, true title screen, renderer/audio/input and clean bounded exit. Qualify the Foundation `NSUserDefaults` bug separately instead of leaving the fallback permanently enabled without a fix.
