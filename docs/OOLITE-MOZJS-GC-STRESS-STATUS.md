# SpiderMonkey 1.8.5 native AmigaOS 68040: object/GC stress diagnostics

18 November 2026. Pinned SpiderMonkey at `OoliteProject/mozillajs-linux@bc12f7124166c199925f3cea37d2732d3704213d`, port patch `0010-gfos-mozjs185-native-68040.patch` and generated big-endian 32-bit target headers. All guest tests use the dedicated AC090 AmigaChrome 68040 test lab. NO production JavaScript engine is claimed from these partial tests.

## Verified passing gates

- `tests/oolite_js_runtime_gate.cpp` in actual native guest: JSRuntime/context/global, standard classes, arithmetic evaluation sum(1..100) = 5050, shell exit 0 (`tests/evidence/oolite-mozjs-68040-guest.log`).
- `tests/oolite_js_stage_gate.cpp` in actual native guest: arithmetic, object literal/property, array literal/index, string literal/length and one explicit `JS_GC(cx)` cycle. All passed with correct values and exit 0 (`tests/evidence/oolite-js-stage-68040-guest.log`).

## Incomplete / stalled larger-workload gates

- Heavy diagnostic (`tests/oolite_js_gc_stress_gate.cpp`): 1,000 object/string allocations, JSON round-trip, exception and three forced GCs. Guest printed only `OOLITE_MOZJS_GC_BEGIN` and made no further output for >60 seconds. The test was not declared successful and did not return its end marker. Original guest stdout was archived to `tests/evidence/oolite-mozjs-gc-heavy-stalled.log`.
- Reduced diagnostic (`tests/oolite_js_gc_small_gate.cpp`): 20 object/string allocations, JSON, exception and a single forced GC. It similarly printed only `OOLITE_MOZJS_SMALL_BEGIN` for >60 seconds without a result. Its guest stdout was archived to `tests/evidence/oolite-mozjs-gc-small-stalled.log`.
- These tests did not emit phase-level progress before entering the JavaScript evaluation. A stall in allocation, JSON, exception handling, or unusually slow guest execution remains possible; **we cannot yet attribute a root cause** or claim GC itself is broken, since the small forced GC unit passed independently.
- For both stalled runs, the test's `run.running` and stdout were copied into the isolated lab's `diagnostics/oolite-mozjs-gc-20261118` folder. Only the dedicated guest was shut down for recovery; the stale running marker was archived. Other teams' guest labs were not touched.
- New segmented diagnostic `tests/oolite_js_features_gate.cpp` compiles/links native 68040 and individually reports JSON round-trip, exception, 20-object allocation loop, 100-element array and explicit GC. Its guest run was **not completed**; another team briefly used the shared lab for its separate `javascript.library` adapter probe (return 77, missing provider), so the queued script was archived rather than interrupting their work. This test must be run in an exclusive guest session. It is NOT a pass yet.

## Immediate next gates

1. Reproduce segmented JS feature test on an *exclusive* 68040 lab; retain each `BEFORE_n` / `AFTER_n` output and bounded timeout.
2. If feature stalls, isolate first failing script into a single native workload; check GC heap/arena alignment, allocation limits, string encoding and big-endian value representation under target. Avoid changing Team 4's `gfopenmulticore.library` SDK and the separate GFOpenSDK `javascript.library` owned by another team.
3. Once genuine JSON, exceptions and allocation stress return exit 0 consistently, integrate original `OOJavaScriptEngine` and Oolite native UI/Universe bindings.

The full original OOOXZManager.m is now separately compiled to a native Amiga object (see `docs/OOLITE-FULL-OXZ-MANAGER-68040-OBJECT.md`), but Oolite itself is **not yet a linked/launching game**.
