# Oolite native 68040 final regression checkpoint — 18 November 2026

This is a combined **real guest** smoke test of existing independently qualified modules using the dedicated AmigaChrome AC090 68040 guest. The guest executed all four binaries consecutively through its `ACAgent` Shell runner, with each `$RC=0`, and reached the end marker. This is not the full Oolite executable and does not imply title-screen or gameplay success.

```
OOLITE_FINAL_SMOKE_BEGIN
OOLITE_MATH_68040 quaternion_w=0.70711 quaternion_z=0.70711 matrix_diag_y=-0.00000 zero_diag_y=1.00000 status=PASS
MATH_RC 0
OOLITE_OXZ_SCHEMA good_parsed=1 good_accepted=1 bad_parsed=1 missing_title_rejected=1 status=PASS
SCHEMA_RC 0
OOLITE_OXZ_PREFLIGHT valid=1 entries=1 missing_rejected=1 escape_rejected=1 duplicate_rejected=1 missing_manifest_rejected=1 status=PASS
PREFLIGHT_RC 0
OOLITE_JS_ABI ptr=4 long=4 size_t=4 u64=8 double=8 long_double=12 big_endian=1 status=PASS
JS_ABI_RC 0
OOLITE_FINAL_SMOKE_END
```

Exact stdout is `tests/evidence/oolite-final-smoke-guest.log`. All test source, checked Makefile targets, upstream-port patches and prior native guest evidence are on `DalsinAI/amigaoolite` main. Dedicated guest only; no other Team 2/Team 4 instance was altered. Full Oolite startup still blocked by real SpiderMonkey 1.8.5 and target NSPR/JSAPI and remaining GUI/rendering integration; see `docs/OOLITE-SPIDERMONKEY-185-NATIVE-GATE.md`.
