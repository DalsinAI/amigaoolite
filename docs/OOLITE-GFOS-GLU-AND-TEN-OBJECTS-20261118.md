# Oolite native GFOpenGPU GLU integration and ten original 68040 engine objects

18 November 2026, 15:12–15:15 UTC. Pinned original Oolite SHA `21978c37b2f11c8664adf3e2062efc1d0c875979` with previous GFOS source overlays already in the workspace. Source dependencies remain pinned to real provider repositories rather than invented headers.

## GLU

`DalsinAI/gfopenrtg/library/modules/glu` provides **real SGI GLU 9.0.3**, including `GL/glu.h` and `libGLU.a`. Its own canonical build.sh ran under the GCC16 m68k 68040 stove and completed RC0, producing `libGLU.a` SHA-256 `3d208b2f14f310045e36332edd39e28fb4053ed3d576745d491694db63aaba02`. This resolves the original `OOOpenGL.m` compile failure at `gluErrorString` without fabricating a declaration or assigning an incompatible old TinyGL API.

New opt-in original Oolite patch `patches/oolite/0003-gfos-native-glu-header.patch` includes genuine `<GL/glu.h>` for GFOS in original `OOOpenGL.m`, leaving other platforms unchanged. `git apply --check` against the actual pinned source workspace passed. The Makefile builds a private patched source copy in OUT and requires genuine GFOpenGPU GLU kit headers/library. `OOOpenGL-gfos-68040.o` compiled (GCC16, -O2 -m68040 -m68881), SHA-256 `7df9f95357d9eb1ff41d395c6c5f72c72ed36dbdce78cc4fdc3cf056b56884ba`.

GCC16 linked a **real native 68040 executable** using canonical SGI libGLU and GFOpenGPU libGL. Tested in actual AC090 AmigaOS 3.2.3 guest:

```
OOLITE_GFOS_GLU enum=invalid enumerant value=invalid value status=PASS
OOLITE_GLU_RC 0
```

Test source `tests/oolite_glu_error_gate.c`, raw output `tests/evidence/oolite-glu-error-guest.log`, `make oolite-glu-native-gate`; reproducible binary hash `e1ef663a56968d636a4ff8eb8c9e05da964fc9912212d14cdbaddd9a7a735ad5`.

## Engine progress

`make oolite-core-ten-objects` now cross-compiles ten original Oolite ObjC source files into 68040 objects: `Universe.m` (with frame hook), `MyOpenGLView.m`, `GameController.m`, SDL `main.m`, `ResourceManager.m`, `OOShipRegistry.m`, `OOSoundSource.m`, `OOCPUInfo.m` and `OOLogHeader.m` (with native GFOS CPU/memory reporting), plus the just-ported `OOOpenGL.m`. There is also the C GFOS Exec system-info module and independent JS bridge objects. A full application link/startup has NOT been demonstrated; `ld -r` earlier failed, many original symbols remain to compile and resolve.

The dedicated guest's separate SDL3 test qualified 18 software-vs-GFOpenGPU scenes (RC0) and cursor lifecycle test completed 30 system/custom cursor create/select/destroy cycles and `SDL_Quit`, RC0 (`tests/evidence/oolite-sdl3-cursor-guest.log`). GFOpenGPU OpenAL AHI Voices passed ALInfo in guest, with its original installed OpenAL module restored after testing. None of these are full Oolite GL scene renders; no title-screen/gameplay claim.

Remaining blockers: compile and link full source graph, ObjC/SpiderMonkey startup and shutdown, GL context/SDL window integration, runtime assets, and completion of safe IPC lifetime handling. Review GCC warning in vendored `stb_image_write.h` before release.
