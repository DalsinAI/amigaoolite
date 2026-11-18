# Pinned upstream Oolite quaternion and matrix math in native AC090 guest

18 November 2026. Pinned OoliteProject/oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

The original Oolite OOMaths.h includes `tgmath.h`; GCC 16.2 68040 target libm does not declare the long-double complex functions `csinl` and `ccosl` expected by target `tgmath.h`, leading to errors when building actual OOOXZManager through Oolite's GUI header chain. `patches/0009-gfos-oomaths-tgmath.patch` selects ordinary `math.h` **only with OOLITE_GFOS**. Non-GFOS code remains unchanged. Patch replay against pristine pinned upstream passed.

`tests/oolite_math_68040_gate.c` includes Oolite's original `OOMaths.h`, `OOQuaternion.h`, `OOMatrix.h` and `OOVector.h` through the original graph. It uses actual `quaternion_set_rotate_about_axis` and `OOMatrixForRotationX` to check expected 90-degree and zero-angle results under native GCC 16.2 `-m68040 -m68881 -noixemul`. Existing `patches/0001-gfos-native-GL-geometry.patch` is also required on the pinned upstream tree so native OpenGPU GL headers replace SDL3 GL headers for this path. The test is a numeric unit, not a whole OXZ manager/game build.

Actual AmigaChrome AC090 68040 guest output:

```
OOLITE_MATH_BEGIN
OOLITE_MATH_68040 quaternion_w=0.70711 quaternion_z=0.70711 matrix_diag_y=-0.00000 zero_diag_y=1.00000 status=PASS
OOLITE_MATH_RC 0
OOLITE_MATH_END
```

Tested binary SHA-256 `d016b61b6dc9ae88d2bab1b5f47c5f135a66d3abc692ab4e2c8aaf1166ff90ca`.

The next actual full `OOOXZManager.m` compile blocker is `jsapi.h` from `GuiDisplayGen.h`. Oolite's pinned build scripts identify **SpiderMonkey 1.8.5**, and link static `libjs_static.a` plus NSPR. Source is OoliteProject/mozillajs-linux at pinned commit `bc12f7124166c199925f3cea37d2732d3704213d`; its original Linux build uses Python 2.7 and NSPR, neither target-qualified in the current m68k toolchain. No replacement JS header or parallel runtime is supplied; full manager and game remain unbuilt.
