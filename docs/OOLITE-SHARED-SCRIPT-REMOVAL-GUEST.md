# Native GFOS Oolite script-object and ship lifecycle: per-ship removal

18 November 2026, dedicated AmiCradle AC090 68040 running AmigaOS 3.2.3. The original pinned 3757-byte `oolite-default-ship-script.js` is loaded unchanged from the guest's `SYS:Oolite/Scripts/`, and invoked by GFOS shared `javascript.library` with native Exec game-task IPC and controlled GNU ObjC ship fixtures.

`src/Core/OOGFOSScriptObjects.{c,h}` adds `OOGFOSScriptObjectDrop(JSTask, scriptGlobal)`, which clears a script object's global reference with canonical GFOS `JS_SetProp(..., NULL)` (= JS `undefined`) on the **JavaScript task**. It does not forcibly run GC or release other possible references; the game owner must join pending script jobs and separately unregister the game's native token before freeing an entity. `OOGFOSScriptObjectEvent` now considers an absent/unregistered script object a normal missing handler, returning `JSERR_OK,called=0` rather than creating a spurious scripting exception. It still returns `JSERR_TYPE` for a present non-callable handler.

`tests/oolite_js_script_removal_gate.c` demonstrates this full controlled lifecycle with two original Oolite script objects hosted by just one JSTask. Both scripts first receive original setup, spawned and death events on distinct native ObjC receivers, then the game task unregisters ship A's generation token. A follow-up asynchronous JS job drops `scriptA`, confirms events for it are no longer called (`called=0`) and `shipA.bounty` is rejected as a stale native token, while the still-live ship B receives another real `shipDied` event and safely resolves its bounty. After its job and completion reply are joined, the second script can still be stopped independently. Both ObjC ship references are freed on the game owner, with game-task port closed.

```
OOLITE_SHIP_SCRIPT_REMOVAL bytes=3757 scripts=2 js_tasks=1 calls=3,4 actions=4,7 rpc=11 done=2 first=1 second=1 called=1 missing=0 droppedA=1 closed=1 status=PASS
OOLITE_CLEANUP_REG4_RC 0
```

Native `make js-script-removal` produces SHA-256 `55cbf47915487fa0e9b0c1c96b0524de9fa417900817b003e5de3cbd0f4f43fd` and passed on AC090 alongside all three prior original-script/ObjC tests after rebuilding, all exit RC0. Complete output: `tests/evidence/oolite-script-removal-four-guest.log`. Direct exploratory GCC executable had different hash due to build-input differences, so no byte identity claim.

**Scope limitations:** The game ships in this test remain controlled selector-compatible ObjC fixtures, not original upstream `ShipEntity.m`. The legacy command elements are recorded, not executed. Full original Universe/OOJSScript ownership, GC-after-drop checks, dead-owner bounded IPC waits and playable game boot are still outstanding.
