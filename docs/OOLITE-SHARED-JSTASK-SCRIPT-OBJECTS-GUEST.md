# Oolite original ship scripts isolated as JS objects on one shared GFOS JSTask

18 November 2026, native AmigaOS 3.2.3 isolated AmiCradle AC090 68040 guest. Canonical shared GFOS `javascript.library` (JavaScriptCore), native GFOpenGNUstep Foundation and existing game-task Exec IPC; original Oolite script unmodified from pinned OoliteProject `21978c37b2f11c8664adf3e2062efc1d0c875979`, staged outside repo at `SYS:Oolite/Scripts/oolite-default-ship-script.js` (3757 bytes). The test does not embed SpiderMonkey.

Earlier two original ships used **two JSTasks**, i.e. two independent JavaScript heaps and default ~512KiB stacks. This scales badly for an Oolite Universe. `src/Core/OOGFOSScriptObjects.{c,h}` now implements reusable shared-heap APIs, designed to be invoked on a **single** JavaScript task, normally through asynchronous `JS_Post`:

- `OOGFOSScriptObjectCreate(task, scriptGlobal, shipGlobal)` validates identifier names, constructs a JS object using GFOS `JS_NewObject`, attaches the registered GFOS host ship wrapper as `script.ship`, and publishes that script object on the task's global. Caller sets script metadata/legacy lists separately before source loading.
- `OOGFOSScriptObjectLoad(task, scriptGlobal, source, length, filename)` wraps the pinned original source in a function invoked with `.call(scriptObject)` so its top-level `this` resolves to **that ship's script object**, not the shared global. It rejects invalid names/sizes and uses no SpiderMonkey API.
- `OOGFOSScriptObjectEvent(task, scriptGlobal, eventName, argc, argv, called)` retrieves the member function using `JS_GetProp` and invokes it by canonical `JS_Call` with `this=scriptObject`; one-shot handlers that delete themselves return `called=0`, not a spurious exception. The returned JS values and wrapper handles are correctly freed via `JS_Free`.

`tests/oolite_js_script_objects_gate.c` uses **one JSTask** for two distinct script objects and native game-owner generation tokens. It loads the same original Oolite default ship script twice with distinct legacy action lists, invokes `shipSpawned`/`shipDied` through `JS_Call`, checks one-shot spawn handler absence, verifies both scripts' names and inherited ship `bounty` field, and confirms native ObjC receiver A had three event calls/four entries and receiver B had three calls/five entries. Eight game-owner Exec messages and one completion passed.

```
OOLITE_SCRIPT_OBJECTS bytes=3757 scripts=2 js_tasks=1 calls=3,3 actions=4,5 rpc=8 done=1 result=1 called=4 missing=0 closed=1 status=PASS
OOLITE_SCRIPT_OBJ_REG3_RC 0
```

Native GCC16 `make js-script-objects` built the executable (SHA-256 `ccec57a5ca9b1ecdbae5546965b2c334200a5e6de9dfe9a779dd73a9eb27cf8b`) and it passed inside AC090 alongside both earlier original-script ObjC tests, all RC0. The exploratory direct build differed in hash from the Makefile build (tool/input path differences); no byte identity is claimed. Complete output: `tests/evidence/oolite-js-script-object-repro-guest.log`.

**Production limitations:** Script objects are created and evaluated on one shared engine task but not yet owned by actual Oolite `OOJSScript`/Universe. The Objective-C ship is a controlled selector-compatible test fixture. Legacy actions are counted by an ObjC method, not parsed or executed as original command strings. An entity-removal cleanup API, bounded dead-owner waits, exact original event argument objects and full game startup remain unimplemented. This is a reusable seam and a native cross-language qualification, **not a playable game**.
