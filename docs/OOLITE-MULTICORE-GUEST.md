# GFOS Oolite: real Team 4 GFOpenMulticore execution in AmigaChrome

**18 November 2026.** Native AC090 68040 AmigaChrome guest, Kickstart 47.2, Workbench 47.100, cores-board-enabled native runtime. The *installed* guest `Libs/gfopenmulticore.library` identified itself as **version 0, revision 6**, string `gfopenmulticore.library 0.6 (01.11.2026) GFOpenMulticore, AmiCradle Team`. Team 4's real public headers and `OMC_RunRange` v0.3+ ABI, not an invented interface, were consumed unmodified from DalsinAI/gfopenmulticore main `f2bcd5688f9e2bad3377149a32387f5da0d9c1c9`. No files were changed in the multicore or SDK repositories.

## Working native Oolite numerical range job — proven board placement

`tests/oolite_rng_omc_range.c` builds with original pinned `OoliteProject/oolite/src/Core/legacy_random.c` (commit `21978c37b2f11c8664adf3e2062efc1d0c875979`). The worker calls actual `MakeRanrotSeed` and `RanrotWithSeed` to process **16 independent chunks × 524,288 iterations = 8,388,608 Oolite RNG iterations**. Each chunk writes one 32-bit digest into a Team 4 `OMC_AllocGrant` page, with aligned `OMC_AddGrant` read/write permission, `OMC_JobInit` and `OMC_RunRange`, then the CPU0 task combines the results in deterministic order. Work never calls Exec, Foundation, DOS or GPU from a board core. Run modes are `OMC_CPU0` and `OMC_ANY`; actual placement is read from `job.omj_Where` (`OMCW_CPU0=1`, `OMCW_BOARD=2`), not inferred from the emulator's `-C` option. `OMC_CoreCount` returned **10**.

All runs returned `OMCERR_OK`, `failed=16`, the **correct Linux/reference checksum `081dedd2`** and guest shell exit code 0. Three repeated invocation results from `tests/evidence/oolite-omc-multicore-guest.log`:

| Run | Forced CPU0 | First OMC_ANY, on board | Second OMC_ANY, on board |
|---|---:|---:|---:|
| Initial cold invocation | 131 ms | **241 ms** | 23 ms |
| Repeat 1 | 201 ms | 35 ms | 51 ms |
| Repeat 2 | 108 ms | 44 ms | 40 ms |
| Repeat 3 | 125 ms | 43 ms | 30 ms |
| Makefile-rebuilt binary | 159 ms | 26 ms | 27 ms |

Guest clock reports `CLOCKS_PER_SEC=1000`. These timings vary due to emulator load and cold-start overhead. **The cold board invocation was slower than CPU0; warmed board runs were typically faster** for this batched workload. Not a statistically controlled benchmark or physical PiStorm result. The explicit `OMCF_BOARD` case was subsequently tested and passed in the guest, while **true no-card fallback remains untested**.

### Reproducibility

Build target: `make rng-omc-range` with a current native Amiga toolchain, `UPSTREAM=/path/to/pinned/oolite`, and **canonical Team 4 header root** `OMC_INCLUDE=/path/to/gfopenmulticore/include`. The Makefile-rebuilt native AmigaOS loadseg executable SHA256 `504050b4be5512744a19d79808492b479c15e5fedc75962e87743ef1a33a8895` **passed in the same real guest**, giving:

```
OOLITE_OMC_LIBRARY version=0 revision=6
OOLITE_OMC_CPU0 call=0 status=0 where=1 failed=16 cores=10 hash=081dedd2 ticks=159 tick_hz=1000
OOLITE_OMC_ANY call=0 status=0 where=2 failed=16 cores=10 hash=081dedd2 ticks=26 tick_hz=1000
OOLITE_OMC_ANY_BOARD_ELIGIBLE call=0 status=0 where=2 failed=16 cores=10 hash=081dedd2 ticks=27 tick_hz=1000
OOLITE_OMC_PASS errors=0
OOLITE_OMC_REPRO_RC 0
```

Exact stdout `tests/evidence/oolite-omc-repro-guest.log`; extended timings `tests/evidence/oolite-omc-multicore-guest.log`. CPU0-only Linux and native guest baselines in `docs/OOLITE-GFOS-PERFORMANCE-BASELINE.md`.

## Integration rules and next gates

- **GFOS / CPU0 owns** Objective-C universe, filesystem, script engine, NSRunLoop, OpenGPU/OpenGfx/OpenAudio calls and all mutable game state. Extra cores handle pure, bounded numeric work with Fast RAM grants. No duplicate thread/worker engine.
- Next Oolite candidates: planet terrain/icosphere generation, procedural texture maths, object transforms and physics kernels. Only offload after proving deterministic results, output ownership and scheduling/transfer overhead; update frame-time and CPU0-responsiveness measurements, not just throughput.
- Team 4's proposed GFOpenSDK PR #72 is **not yet merged**; that host native-jobs API is separately validated on daletop at host level but was **not used** for this 68k `OMC_RunRange` test.
- Confirm the multicore board under different core counts, absent-board CPU0 fallback, cancellation/timeouts and grant isolation; repeat on physical PiStorm before asserting hardware performance. No Oolite full gameplay or video rendering proven by this test.

## Explicit board-only execution (follow-up)

`tests/oolite_rng_omc_range.c` now explicitly sets `job.omj_Flags=OMCF_BOARD` for its `BOARD_ONLY` case, while keeping `OMC_CPU0` and `OMC_ANY` tests. The Makefile-rebuilt native binary (SHA256 `21937fe08f8f0c8986867d758323d2275801929731a5c07bb626684404d945b8`) ran inside the same guest and returned:

```
OOLITE_OMC_LIBRARY version=0 revision=6
OOLITE_OMC_CPU0 call=0 status=0 where=1 failed=16 cores=10 hash=081dedd2 ticks=99 tick_hz=1000
OOLITE_OMC_ANY call=0 status=0 where=2 failed=16 cores=10 hash=081dedd2 ticks=30 tick_hz=1000
OOLITE_OMC_BOARD_ONLY call=0 status=0 where=2 failed=16 cores=10 hash=081dedd2 ticks=22 tick_hz=1000
OOLITE_OMC_PASS errors=0
OOLITE_OMC_BOARD_ONLY_RC 0
```

Exact evidence: `tests/evidence/oolite-omc-boardonly-guest.log`. The board-only flag worked; the output matched CPU0. No PiStorm hardware performance claim.

## Final strict-placement gate

A final strengthened source revision now treats a `BOARD_ONLY` job returning `omj_Where!=OMCW_BOARD` as a **test failure**, not merely a diagnostic. The cross-compiled binary SHA256 is `28a1aafb2a6053eb67aa5ce87f21f6eaba013d7da178ac63dbbc8d0c9b77e949`. The actual native guest ran that new build and passed:

```
OOLITE_OMC_LIBRARY version=0 revision=6
OOLITE_OMC_CPU0 call=0 status=0 where=1 failed=16 cores=10 hash=081dedd2 ticks=113 tick_hz=1000
OOLITE_OMC_ANY call=0 status=0 where=2 failed=16 cores=10 hash=081dedd2 ticks=26 tick_hz=1000
OOLITE_OMC_BOARD_ONLY call=0 status=0 where=2 failed=16 cores=10 hash=081dedd2 ticks=29 tick_hz=1000
OOLITE_OMC_PASS errors=0
OOLITE_OMC_FINAL_GATE_RC 0
```

This latest test is the authoritative source build and raw test output `tests/evidence/oolite-omc-final-guest.log`. Earlier guest logs remain for cold-start and repeated performance context. Full game rendering is still NOT tested.
