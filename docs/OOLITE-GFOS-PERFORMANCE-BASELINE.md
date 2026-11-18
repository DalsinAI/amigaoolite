# Oolite GFOS CPU0 baseline for Team 4 multicore offload

**18 November 2026, validated in native AmigaChrome guest** (AC090 A1200, GCC16 68040/FPU, Kickstart 47.2 / Workbench 47.100). No PiStorm hardware measurement has been made. The guest was run with AmiCradle native emulator, which exposes a cores-board flag, but **this benchmark uses CPU0 only**: no OMC job submissions, no board acceleration, no GFOpenGPU job.

## Real upstream game workload

`tests/oolite_rng_cpu0_baseline.c` compiles and calls **actual upstream Oolite** `src/Core/legacy_random.c`, specifically `MakeRanrotSeed` + `RanrotWithSeed`. Sixteen independent RANROT seed streams (safe per-chunk data, no global RNG state), each **524,288 steps**. Total **8,388,608 RNG steps**. Each chunk emits a 32-bit FNV-style checksum, the main task combines 16 hashes deterministically in order. The same test is built and run on Linux and on m68k, and returns exact same checksum `081dedd2`. All 16 individual chunk outputs matched too. This is a **correctness reference workload** for CPU0 versus multicore board and possible later native host-ISA jobs, not yet a demonstrated offload.

## Guest and host evidence

`tests/evidence/oolite-cpu0-rng-8m-guest.log` is the final clean Makefile-built binary run, reporting `OOLITE_CPU0_BASELINE chunks=16 iterations=524288 checksum=081dedd2 clock_ticks=96 ticks_per_sec=1000`, shell `OOLITE_CPU0_REPRO_RC 0`. An earlier direct build took **122 guest clock ticks** (clock tick 1 ms). Results 96 ms and 122 ms illustrate variability in the busy shared emulator environment; they are not performance guarantees or PiStorm timings. Host Linux reference SHA output and its own clock ticks are in `tests/evidence/oolite-cpu0-rng-host.log`; host and guest wall/CPU clocks are not directly comparable. Guest binary produced by repo `make rng-cpu0-baseline` SHA256 `6faef40bc6c36b1c598cd6c9d5632502ca7daf6dc8ac00f26fab9b528a08a4b2`, tested inside guest.

## Performance and ownership rules

1. **GFOS/Exec CPU0 owns the OS, UI/event loop, Foundation/ObjC world graph, SDL/OpenGPU submission, audio, IO and Oolite mutable state.** Never call Foundation or Exec from extra-core worker functions.
2. **Team 4's `gfopenmulticore.library`** owns 68k worker dispatch, Fast RAM grants, range execution and completion signals. Do not invent a second worker runtime or a private card ABI. First Oolite offload uses `OMC_RunRange` (only if `OMC_HAS_RANGE`) or `OMC_Submit` for individual chunks; completion via `OMC_SetSignal` (only if `OMC_HAS_JOBCALLS`). Use `OMC_ANY`, and preserve CPU0 output parity / fallback on any unavailable-card error.
3. **Chunk memory is explicit.** A worker receives its own seed state, counter and a cache-line-aligned Fast RAM result slot, via canonical `OMC_AddGrant`/`OMC_AllocGrant` calls. It must not mutate the global RNG or Oolite NSObject instances. CPU0 combines hashes after all workers complete; only then modify the Objective-C world.
4. **Measurement conditions.** Record whether board cores were actually detected via `OMC_CoreCount`, `omj_Where==OMCW_BOARD`, requested core count, actual jobs submitted and completed, errors/timeouts/cancellation, wall/CPU time, transfer cost and CPU0 responsiveness. Compare 1/2/N cores against the SAME CPU0 result. A cores-board emulator flag `-C` alone does not prove an offloaded job ran.
5. **Native host-ISA acceleration** can follow Team 4's GFOpenSDK PR #72 (`cores/acn_job.h`, host x86-64/ARM64 native jobs, `mitchy.library`), only after the SDK version is merged and verified. Avoid requiring AC_NATIVE=1 for normal game startup.
6. **Real PiStorm benchmark is a separate gate**: same test on physical PiStorm and any available hardware core combinations, recording cache-coherent writes, task latency and frame-time impact. Do not extrapolate emulator 96/122 ms into PiStorm frame performance.

## Next development gates

- Confirm Team 4 PR #72 is merged and that `gfopenmulticore.library` binary/SDK match current main; run an actual range job in a confirmed cores-card guest with CPU0 fallback and the 16-chunk checksum. Record per-core work placement and overhead.
- Profile **actual Oolite gameplay** after startup: planet mesh generation, procedural textures, object transforms and collisions, JavaScript/OOXP overhead, shader/GPU submission and audio. Offload only operations where compute time exceeds dispatch/grant overhead. Use GFOpenGPU/OpenGfx/OpenAudio, not new render/audio stacks.
- The original Oolite `OOPListParsing.m` source has now also passed actual guest tests with a GFOS-specific ordinary AmigaDOS file patch, and remains a CPU0/IO task.

This is a verified **CPU0 correctness and timing baseline**. No claims of game playability or Oolite multicore acceleration are made.
