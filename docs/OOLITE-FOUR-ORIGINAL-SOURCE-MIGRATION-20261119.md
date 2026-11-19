# Original Oolite OOJSScript: actual source migration

19 November 2026. Pinned original Oolite 1.93.1; GCC16.2 68040.
This checkpoint reuses the canonical GFOpenJavaScript v2 JSTask and
existing OOGFOSScriptObjects/ScriptInvoke; no legacy SpiderMonkey
runtime, context, jsval or jsid crosses this new source boundary.

- Real original OOJSScript Objective-C class replacement:
  src/Core/original-replacements/OOJSScript.m
- Canonical shared-JSTask owner interface:
  src/Core/original-replacements/OOJSScriptGFOS.h
  src/Core/OOGFOSOriginalScriptBridge.{c,h}
- Compiled original object Core_Scripting_OOJSScript_m.o SHA256:
  4be05a50f49e91a3ebf428f1ab12622a115e0a9434fa53367fec80f094d0ed7b
- Compiled bridge object OOGFOSOriginalScriptBridge.o SHA256:
  a03e9a930e05afcab81adcaa9d9def5ecad419341eff00199ca883b812efb865
- Native m68k nm: **zero** undefined JS_* from the migrated OOJSScript object.
- The 242-original-object corpus, with all four authentic rebuilt original
  objects substituted, now has **49** legacy-dependent objects and **85**
  distinct legacy JS_* externs; unknown=0, overlap=0.
  Reproducible JSON:
  tests/evidence/oolite-original-242-object-jsapi-85-49-20261119.json.
- Four previously recorded native AC090 script cleanup regression tests
  passed, but they are not a full original OOJSScript+Universe integration
  test. No playable game binary exists.

The original OOJavaScriptEngine and remaining 49 legacy-dependent
translation units still require real source migration. Do not
link libjs_static.a or claim this object-only gate proves a complete
SpiderMonkey-free Oolite executable.
