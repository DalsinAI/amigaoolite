# Oolite RC1 — CURRENT VERIFIED ENGINEERING STATE

**Last refreshed:** 2026-11-19 UTC. **Read this FIRST in every session.**
This document is the continuity pointer, not a claim that RC1 is playable.
The repository and its real build/test artifacts are authoritative; refresh
the pinned heads and audit before starting new work. Update this document
in the SAME GitHub change that advances a release milestone.

## In flight (Team 5, 19 Nov 2026)

- Branch `team5/gfos-runtime-foundation-20261119`: the game on javascript.library's script task (direct JS calls; gate PASS on AC090), and eight GFOS toolchain/Foundation faults fixed (exceptions need `-fexceptions` at compile **and link**; Foundation rebuilt for 68040+FPU with native exceptions; user/group/uid shim; Amiga path roots; absolute argv[0]). NSUserDefaults, NSLog and time zones now work. See `docs/GFOS-RUNTIME-FOUNDATION-20261119.md`.
- Dale's route, 19 Nov: SpiderMonkey's code and libraries go completely; its API *names* may stay, through a header implemented on javascript.library. Team 5 owns `OOJavaScriptEngine.m` and that header; please don't start on either without a word in `teams/team5/`.
- Branch `team5/gfosjsapi-header-20261119`: that header, `src/Core/gfosjsapi/` (jsapi.h, jsdbgapi.h, jsxdrapi.h; gfosjsapi.c over oojsl.c, the only file that sees `proto/javascript.h`). All 53 upstream JSAPI sources compile against it (`tools/gfosjsapi-compile-check.sh`); all 108 `JS_*` symbols they need are defined; unit gate 0 failures on AC090 (`tools/build-gfosjsapi-gate.sh`, `tests/evidence/gfosjsapi-gate-20261119-2228.log`). About 80 µs per JS→C crossing, 100 µs per C→JS call. Not yet linked into Oolite. Known gap: top-level `var`/`function` in a script run against an object stay local (SpiderMonkey puts them on the object).
- 20 Nov: PRs #4, #5, #6 merged. Branch `team5/oolite-define-twice-20261120`: the game **runs** in the lab: "Loading complete" in about 20 s, the game loop ticking, the title screen's demo ships drawn textured with OpenGL 4.3 (virgl). Fixed on the way: a property defined twice (Ship), JS_ClearScope for Oolite's JavaScript reset, the game loop without NSRunLoop (patch 0019), async work on the game task (patch 0021). Open: no text on the title screen; theme music (OpenAL streaming). See `docs/OOLITE-FULL-LINK-RUN-20261119.md`.

## Goal and acceptance contract

Deliver original upstream Oolite 1.93.1 as a playable native AmigaOS 3.2.3
AC090 68040 application using the EXISTING GFOS/OpenUp components:
canonical GFOpenJavaScript v2 (JavaScriptCore), GFOpenGPU/RTG SDL3+GL,
OpenAL/AHI and native Foundation. **No SpiderMonkey runtime linked.**
Acceptance: full original game link with no legacy JS_* externs, normal
title/menu, start flight, functional controls, real rendering, audio,
original OXP/worldScripts, docking, save/reload, repeat launch/exit.
Do not publish an RC tag until game runtime acceptance is actually proved.

## Pinned sources and current verified GitHub heads

- DalsinAI/amigaoolite **29149c4aec7ea2884729a6de4fd9534c93ae0fbc** (main; baseline before this update)
- DalsinAI/gfopenjavascript **45f9bda6d39ed8ac55a49e4b7a9f13ef5dca57bc** (stable v2 main)
- OoliteProject/oolite **21978c37b2f11c8664adf3e2062efc1d0c875979** (upstream original)
- Existing historical release v1.93.1-gfos-rc0-checkpoint-20261119
  is **NOT PLAYABLE** and must not be treated as RC1.
- GitHub is the code source of truth. No local Git commits. Verify heads
  before pushing; use non-force updates.

## Already implemented — REUSE, DO NOT REBUILD

There are **43 OOGFOS* source/header files** under src/Core, plus
original-replacements and numerous qualified guest gates.
Relevant actual modules:
- OOGFOSJavaScript.*, OOGFOSJSValues.*,
  OOGFOSJSClassBootstrap.*: v2 library/task, value ownership/conversion,
  class/prototype bootstrap.
- OOGFOSScriptObjects.*, OOGFOSScriptInvoke.*,
  OOGFOSScriptEventInvoke.*, OOGFOSWorldScripts.*:
  shared JSTask, original UTF-16 OXP script loading, inter-script calls,
  per-script object method invocation and owned results.
- OOGFOSScriptReset.*, OOGFOSGameDispatch.*,
  OOGFOSGameFrame.*, OOGFOSShipEntityBackend.{c,m,h},
  OOGFOSShipTokens.*: reset, game-task RPC pumping, entity-selector
  projection, token validation, lifecycle.
- OOGFOSJSVector.*, OOGFOSJSQuaternion.*,
  OOGFOSRegExp.*, OOGFOSCredits.*: native scripting classes/bridges.

Native AC090 tests already passed for original Oolite JavaScript ship
scripts, WorldScripts, Vector3D/Quaternion, object/script lifecycle,
exceptions, 12 isolated JSTasks, original Objective-C regex selectors,
and credits including uint64_t. See tests/evidence and the cited docs.
These are valid **component tests**, not a game startup.

Four authentic original Objective-C translation units are REBUILT as
GFOS-facing original objects:
Core_OORegExpMatcher_m.o, Core_OOStringParsing_m.o,
Core_Scripting_OOJSFont_m.o and Core_Scripting_OOJSScript_m.o.
An additional original scripting interface, OOJSFont's
defaultFont.measureString, now has a separately compiled source-level
GFOS C replacement plus an Objective-C owner callback that calls the
real upstream OOStringWidthInEm; guest font RPC test and five existing
game-dispatch regressions passed. The font measurement native guest
test uses a controlled width fixture, not a running Oolite title UI.
See tests/evidence/oolite-original-font-native-guest-20261119.log and
tests/evidence/oolite-font-dispatch-five-native-regressions-20261119.log.
This font replacement has NOT yet been wired into the original global
setup in OOJavaScriptEngine.m; no full-game link is claimed.
Original world/global OOJSScript semantics now have a second native
integration pass: shared JSTask script objects accept NO ship wrapper
for global scripts, and metadata properties support real read-only
descriptors, ordinary mutable values and owned property retrieval.
AC090 native proof in
tests/evidence/oolite-worldscript-metadata-original-semantics-native-20261119.log.
Original OOJSScript.m has NOT yet been replaced by this C adapter.
Run make jsapi-four-original-replacements-audit with the pinned
headers and the entire **242-object** baseline. Last verified result:
**242 objects / 49 still using JS_* / 85 unique historical JS_* externs**
(unknown=0, overlap=0), evidence:
tests/evidence/oolite-original-242-object-jsapi-85-49-20261119.json.
This measures **legacy references in original objects**, NOT how
much GFOpenJavaScript replacement functionality already exists.

## The REAL integration gap

The original complete engine has NOT been linked with GFOS alone.
Legacy original OOJavaScriptEngine.m still owns SpiderMonkey runtime
and context; the rebuilt OOJSScript.m is not yet wired to the full
original engine; remaining OOJS classes still
invoke historical APIs. The existing adapter functionality must be
wired at the original game call sites, preserving original script
identity and the SAME shared JSTask for worldScripts, not replaced
with new standalone fixtures. Integrate native task ownership,
host-object registration, event dispatch, reset and error handling.
The source migration audit must count genuine rebuilt original
translation units, not invented JSAPI symbols or omitted objects.

The historical source/port/build/full-link.mk target is NOT a
production GFOS link: it explicitly uses FULL_JS185_LIB /
libjs_static.a. Keep as cross-reference ONLY. Extend this existing
242-source machinery for a GFOS-only link; do not start anew.

## Highest-priority next EXECUTABLE engineering step

1. Migrate original src/Core/Scripting/OOJavaScriptEngine.m
   using existing OOGFOS* implementations, and connect the already
   rebuilt OOJSScript.m class to its original callers.
   Use pinned GCC16/SDK; no new interpreter or JSAPI shim.
2. Preserve singleton/reset/notifications and original script
   method selectors, manifest/name, shared globals, running-script
   stack, UTF-16 loading and owned-value semantics; use existing
   game-task RPC pump. No reimplementation of proven components.
3. Inject rebuilt originals into the exact 242-object link set,
   rerun jsapi_link_audit.py, and eliminate concrete failures in
   dependency order. Publish each source integration with object
   hashes, compiler results and actual symbol-count changes.
4. Link all original game units without libjs_static.a, then run
   original title/flight on AC090, fix genuine runtime failures,
   and only then publish a playable RC.

## DO NOT REDO / traps from previous sessions

- Do not re-clone or re-design SDK, JSVector/Quaternion,
  WorldScripts, IPC/token bridge, regex, credit bridge, SDL3 or GLU.
- Do not spend the release session on JS_Stop, Template JIT,
  v3 cancellation, extra standalone probes, broad GCC optimisation
  or another architecture plan. These are separately tracked.
- A PRIVATE m68k trap-poll JavaScriptCore archive and executable were
  cross-built under /tmp/gfopenjavascript-v3-20261119/trap-poll.
  **No native JS_Stop success recorded.** Stable JS v2 stays pinned.
- Do not mistake 98 unique original JS_* names for 98 percent
  incomplete; existing replacement functions are substantial. Nor
  do isolated adapter passes mean original game integration is done.
- Do not repeat header archaeology: genuine pinned SDL3 3.4.14,
  GLU 9.0.3, legacy compile-time JSAPI headers and GCC16 ABI
  tgmath header were already recovered and the three-object audit PASSED.
- No mock game, splash preview or -help exit counts as playable.

## Existing daletop locations — verify before use

- Current port:
  /run/media/da1ek/DevDrive/AmiCradle-labs/team1-oolite-spidermonkey-elimination-20261119/port
- Pinned upstream:
  /run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1/source/upstream
- Historical 242-source rules:
  /run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1/source/port/build/full-link.mk
- Original 242 objects:
  /run/media/da1ek/DevDrive/AmiCradle-labs/oolite-rc1/evidence/rc1-title-20261119/build/oolite-complete-objects
- Last four-replacement reproducible audit:
  /tmp/oolite-rc1-four-original-20261119/
- GCC16 prefix:
  /tmp/atlas-batch10-20261117/stove-isolated/prefix
- Isolated native test instance:
  /run/media/da1ek/DevDrive/AmiCradle-labs/team1-oolite-jsapi-rc1-20261119
- Authentic recovered SDL3 headers:
  /tmp/oolite-rc1-sdl3-20261119/SDL3-3.4.14/include
- Authentic recovered GLU headers:
  /tmp/oolite-rc1-glu-20261119/glu-9.0.3/include
- Pinned SpiderMonkey headers ONLY for legacy reference compile:
  /tmp/oolite-rc1-mozjs185-20261119/target-headers

Scratch paths may disappear. Recover pinned inputs only as needed;
never overwrite the stable guest or stable JS library v2.

## Working discipline

Read THIS document and verify GitHub heads at session start.
Report only release milestones or actual blockers. Update THIS
document with every meaningful source/build improvement and push.
A new chat must not reconstruct progress from conversation history.
This is an ENGINEERING CONTINUATION, not permission to restart.
