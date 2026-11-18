# Oolite SpiderMonkey 1.8.5: real native AmigaOS 68040 JavaScript execution

18 November 2026. Oolite pinned engine: `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`. **Separate, exact** original JavaScript source: `OoliteProject/mozillajs-linux@bc12f7124166c199925f3cea37d2732d3704213d`. No replacement JS engine or fabricated SDK was used.

## Evidence and result

The matching upstream SpiderMonkey 1.8.5 source was patched via `patches/0010-gfos-mozjs185-native-68040.patch` (applies to the *mozillajs-linux* source tree, NOT the Oolite engine tree). Patch replay against clean pinned source passed for all seven modified files. A GCC 16.2 native 68040 cross-build completed successfully and produced actual Amiga object code in `libjs_static.a` (2.2 MB). A standalone C++ `tests/oolite_js_runtime_gate.cpp` was compiled and linked against this archive to an actual 1.9 MB AmigaOS executable. It does not call GNUstep, OpenGPU or Team 4's multicore SDK.

The native executable was staged in the **isolated AC090 AmigaChrome 68040 guest**, and SHA-256 on host and staged binary matched. The guest initialised `JSRuntime`, `JSContext`, a JavaScript global object, standard classes, then used `JS_EvaluateScript` to sum 1..100 via a JavaScript loop. Actual guest console output (unmodified copy in `tests/evidence/oolite-mozjs-68040-guest.log`):

```
OOLITE_MOZJS_BEGIN
OOLITE_MOZJS_68040 runtime=1 context=1 global=1 eval=1 value=5050 status=PASS
OOLITE_MOZJS_RC 0
OOLITE_MOZJS_END
```

Archive SHA-256: `50eebc624798de7e89f9207c425facbd9dde45ebb935e78413e17f9dd9155f92`.
Native executable SHA-256: `483ea8a1b5987dd10279a8b5304b7af4907349aa8254fccb689d2d6180d9ad78`.
Patch SHA-256: `efc803fc11ef905c617f1f8c1036b2936547ff0a5cf342112edfe0caeacec9ee`.

## Exact architecture decisions

- Interpreter-only first gate. `--disable-methodjit --disable-tracejit` avoids all host executable-memory assumptions on a no-MMU 68040. Oolite still uses the upstream SpiderMonkey 1.8.5 **API**.
- Single-thread first gate. `--disable-threadsafe` and `--disable-ctypes` were selected explicitly for the first CPU0 engine qualification. An upstream configure bug defined `JS_THREADSAFE` even for `--disable-threadsafe`; patch 0010 fixes both `configure` and `configure.in` so generated `js-config.h` has `/* #undef JS_THREADSAFE */`. This does **not** claim Oolite's fully threaded scripting integration is ready.
- Big-endian, 32-bit target ABI. The host `jscpucfg` generator must be built with `HOST_CXXFLAGS='-O2 -DCROSS_COMPILE -DFORCE_BIG_ENDIAN'`, otherwise the generated header incorrectly reports the Linux *host* as little-endian. Verified guest ABI independently: 32-bit `void *`, `long`, `size_t`; big-endian. Generated JS headers report `JS_BYTES_PER_WORD=4`, `IS_BIG_ENDIAN=1` and `IS_LITTLE_ENDIAN` undefined.
- GCC 16 compatibility. The old `configure` compiler probe uses implicit-int C, so its **target** C flags require `-std=gnu89 -Wno-error=implicit-int -Wno-error=implicit-function-declaration` for initial configure. The source's `build/hcc` and `build/hcpp` wrapper files were not executable in the pinned checkout and had to receive executable permissions in the disposable stage. For host tools (`nsinstall`, `jscpucfg`) use native `gcc/g++`, never m68k cross-generated host executables.
- GNU C++ ABI and data alignment. The 68040 GCC target uses two-byte pointer alignment; upstream `StackSegment` and `JSScript` size static assertions failed. Patch 0010 adds narrowly guarded Amiga/m68k padding, without suppressing assertions or modifying non-Amiga targets.
- Amiga VM interface. This target libc declares `mmap`/`munmap` but **does not provide linkable implementations** (verified by a direct native linkage probe). Patch 0010 reserves the JS stack with `malloc`/`free` and supplies aligned GC chunks with overallocated storage. The initial runtime/GC allocation passed, but extended GC stress, capacity management and allocation exhaustion must be qualified before general use. No fake `MAP_FAILED` success is asserted.
- Native stack inspection. Linux `pthread_getattr_np` is missing; use Amiga Exec `FindTask(NULL)->tc_SPUpper` for downward-growing native stack. This compiled with the NDK headers and executed in the guest.
- AmigaOS does not support the build's Linux `-fPIC` use. Static library construction was done with `DSO_PIC_CFLAGS='' ASFLAGS=''`. Guest runtime is linked with `-noixemul`, `-m68040` and `-m68881` against the pinned GCC 16.2 environment.

## Reproduction requirements

Use a **separate** pinned `mozillajs-linux` checkout and apply `patches/0010-gfos-mozjs185-native-68040.patch` there. Use an isolated Python **2.7** runtime on the build host; this session obtained Python 2.7.18 from `python:2.7-slim` in Docker and unpacked only its runtime into `/dev/shm/oolite-py27`; no system-level Python replacement. Set its `PYTHONHOME` and `LD_LIBRARY_PATH` to its isolated prefix. Configure in an out-of-tree `js/src/gfos-cross-build` with:

```
--build=x86_64-pc-linux-gnu --host=m68k-amigaos --target=m68k-amigaos
--disable-shared-js --disable-methodjit --disable-tracejit
--disable-tests --disable-ctypes --disable-threadsafe
```

Environment: `CC=m68k-amigaos-gcc`, `CXX=m68k-amigaos-g++`, `AR=m68k-amigaos-ar`, `RANLIB=m68k-amigaos-ranlib`, `PYTHON=<isolated-python2.7>`, `LIBS=-lpthread`. Target `CFLAGS='-O2 -std=gnu89 -Wno-error=implicit-int -Wno-error=implicit-function-declaration -m68040 -m68881 -noixemul'`; `CXXFLAGS='-O2 -m68040 -m68881 -noixemul'`. Add the existing target NDK include path to target C++ builds. Make overrides: `HOST_CC=gcc HOST_CXX=g++ HOST_AR=ar HOST_RANLIB=ranlib HOST_CFLAGS=-O2 HOST_CXXFLAGS='-O2 -DCROSS_COMPILE -DFORCE_BIG_ENDIAN' HOST_LDFLAGS='' DSO_PIC_CFLAGS='' ASFLAGS=''`. Link the test with target `m68k-amigaos-g++` and the generated `libjs_static.a -lpthread -lm -lsocket -lz` using normal target-group linking. `js-config.h` and `jsautocfg.h` are **generated ABI artifacts**, not checked-in dummy headers.

## Gaps before complete Oolite startup

- Only a single, deterministic JavaScript arithmetic test has passed to date. Garbage-collection stress, exceptions, objects and Oolite JS binding APIs need further testing.
- No target NSPR is available/qualified; `JS_THREADSAFE` is **off** in this initial build, whereas upstream Oolite's standard Linux build enables threads and uses NSPR. Do not mix threaded and single-threaded SpiderMonkey ABI definitions.
- No actual full `OOOXZManager.m` or GUI/Universe linked native executable has been qualified against the new runtime. The OXZ extraction/backend and OpenGPU/native math passes remain separately verified; bringing them together is the next engine integration gate.
- Allocation and stack handling need real Amiga hardware and low-memory stress qualification; do not label the current binary a production release.

Keep this JavaScript runtime initially CPU0-owned; numerical side jobs can use Team 4's canonical multicore API. No changes to their SDK.
