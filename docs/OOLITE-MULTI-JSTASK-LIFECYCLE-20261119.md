# Original Oolite owner: many reusable JavaScript tasks

19 November 2026. Canonical GFOpenJavaScript v2, AmiCradle AC090 guest,
AmigaOS 3.2.3, native m68k 68040 binary.

## Native task ownership qualification

The game can create, own and destroy many independent Javascript contexts
through existing JS_NewTask / JS_EndTask, without any shared-core shutdown.
This was tested with exactly 12 Oolite-owned OOGFOSJS task entries:

- All 12 initialized, separately evaluated JavaScript, and returned 42.
- Each independently retained its own oxpIdentity/oxpState global, verified
  for all 12. Each is an isolated JSC VM with distinct global object.
- Oolite ended one task while the other 11 remained alive. A newly created
  task in that slot had **no inherited global**.
- Another still-live task retained its original world script state.
- All 12 entries were explicitly cleaned up via OOGFOSJSClose /
  JS_EndTask, leaving the shared JavaScriptEngine resident for other
  applications.

Native guest evidence:
    OOLITE_OWNED_TASKS created=12 expected=12 lastCode=0 cleaned=12 status=PASS
    OOLITE_OWNED_TASKS_RC 0

Log: tests/evidence/oolite-12-owned-js-tasks-ac090-20261119.log

## Important distinction

Original Oolite historically uses one SpiderMonkey runtime/main context
and shares its worldScripts namespace between scripts. The native test
proves Oolite **can** own 12 isolated JSTasks without harming one another,
but DOES NOT authorize changing existing OXP compatibility semantics to
one isolated task per OXP.

Native 12-task create/use/destroy is verified with v2, **not** a verified
JS_Stop for runaway JavaScript. v3 cancellation remains on a deliberately
unmerged experimental branch in DalsinAI/gfopenjavascript; both initial
native loop-cancellation experiments failed. An infinite loop may block
JS_EndTask; production code must not assert end-task safety for a
non-cooperative script until that interrupt mechanism passes.

Original source symbol audit remains 100 unique JS_* over 52 objects, and
original Oolite not linked or playable.
