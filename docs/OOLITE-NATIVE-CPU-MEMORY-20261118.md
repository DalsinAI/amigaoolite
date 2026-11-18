# Oolite GFOS native CPU and memory report — 68040 qualification

18 November 2026, 15:10–15:14 UTC. Native AmigaOS 3.2.3 AC090 guest and pinned OoliteProject/oolite original `21978c37b2f11c8664adf3e2062efc1d0c875979` with pre-existing GFOS source patch set in the working tree.

Oolite's original `OOLogHeader.m` calls `OOCPUDescription()` and `OOSystemMemoryStatus()` but original `OOCPUInfo.h/.m` only provides them for x86/Linux/Windows. We **do not set OOLITE_LINUX on 68040** or attempt to execute x86 CPUID. The opt-in source patch `patches/oolite/0002-gfos-native-cpu-memory-report.patch` adds GFOS declarations/Objective-C implementations, identifies platform as `AmigaOS (GFOS)`, and reports `m68k (GFOS AmigaOS)`. `OOCPUCount()` reports **1 game-owning CPU**; the GFOpenMulticore board is not automatically a set of native ObjC simulator threads and remains opt-in. `src/Core/OOGFOSSystemInfo.{c,h}` performs real AmigaOS Exec `AvailMem(MEMF_ANY)` and `AvailMem(MEMF_TOTAL)` calls in **C only**. The public header contains no Exec types, avoiding the `BOOL` conflict with GNUstep ObjC.

`make oolite-core-nine-objects` now builds **nine original Oolite ObjC application modules**: `Universe`, `MyOpenGLView`, `GameController`, SDL `main`, `ResourceManager`, `OOShipRegistry`, `OOSoundSource`, patched `OOCPUInfo`, patched `OOLogHeader`, plus the new GFOS native C system-info provider. All compile successfully using GCC 16.2 `-O2 -m68040 -m68881` and real pinned upstream source/headers. The patched original tree is copied into `OUT`; the vendor checkout is not edited by the build. Patch applies with `git apply --check` at pinned SHA.

The C system-info gate was built at GCC 16.2 `-Wall -Wextra -Werror`, run **in the actual AC090 native guest**, and passed RC0:

```
OOLITE_GFOS_SYSTEM_INFO total=662691776 free=512461176 total_mb=631 free_mb=488 status=PASS
OOLITE_SYSINFO_RC 0
```

Actual raw output is `tests/evidence/oolite-gfos-system-info-guest.log`. A `make -B` rebuild of the nine original object targets and system-info test completed RC0. SHA256 of native outputs at final rebuild:
- `OOCPUInfo-gfos-68040.o`: `79e091b17b5f9ce9ce146b11d18afe00ab40d417c7c3001a38bb6462c5d17ad1`
- `OOLogHeader-gfos-68040.o`: `8d1adacea7c510e9348d93c6209d7262d5c82ec88e806575ca26cbcfed16e870`
- `oolite-gfos-system-info-68040`: `0720e8558e5e9ca720ef00d31fc22685d492a5ecf82853d0d4abbf14ed8d7a20`

Limits: the memory count represents what AmigaOS Exec exposes to the guest (not physical PC RAM); `OOCPUCount=1` is appropriate for CPU0 ownership pending proper GFOpenMulticore integration. No live Oolite startup, full app linkage or gameplay is proven. Next: GLU (`OOOpenGL.m` needs real `gluErrorString`), remaining engine objects and complete application linking. Existing compiler warning in original `stb_image_write.h` retained for review.
