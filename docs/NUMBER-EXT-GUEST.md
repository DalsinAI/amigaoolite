# Oolite native NSNumber category — GFOpenGNUstep guest validation

18 Nov 2026. Pinned original OoliteProject/oolite upstream at 21978c37b2f11c8664adf3e2062efc1d0c875979, `src/Core/NSNumberOOExtensions.m` compiled unmodified with GCC16 `-m68040 -m68881` against native GFOpenGNUstep 1.31.1. Linked with the guest-tested native Foundation/ObjC/ffi archives. AmigaOS native loadseg binary SHA256: `4c5834269811f4348673de62fb44fd5745e8635c63da1da58991c11e184d6636`.

Actual native AC090 68040 AmigaChrome guest shell evidence:

```
OOLITE_NUMBER_EXT_BEGIN
OOLITE_NUMBER_EXT_PASS tests=4 failed=0
OOLITE_NUMBER_EXT_RC 0
OOLITE_NUMBER_EXT_END
```

Checks two float types, integer rejection and a GNUstep boolean NSNumber, via actual upstream `[NSNumber oo_isFloatingPointNumber]` / `[NSNumber oo_isBoolean]` methods. The upstream implementation is **not copied into this repo**, stays at pinned upstream commit. This is Foundation component smoke evidence, not multicore, gameplay or rendering proof. Test source `tests/oolite_nsnumber_gate.m`, guest stdout `tests/evidence/oolite-number-extensions-guest.log`.

Related integration requirements: `docs/GFOS-OPENUP-MULTICORE-INTEGRATION.md` and Team 4 coordination issue https://github.com/DalsinAI/gfopenmulticore/issues/31 . CPU0 retains Objective-C Foundation objects; accelerator cores receive only pure numeric kernels via canonical library, not these Objective-C methods.
