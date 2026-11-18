# Oolite GFOS platform offload strategy — GFOpenMulticore, AkikoPlus, GFOpenGPU

18 November 2026. Source-grounded engineering plan, not a claim of completed Oolite runtime offloads. Original Oolite Objective-C application currently has 41 pinned original source units compiled under the standalone native 68040 gate and an actual SDL3-branded original-artwork preview has rendered on AC090; complete engine application link/startup remains outstanding.

## Grounded platform interfaces

* GFOpenMulticore: `DalsinAI/gfopenmulticore` README and `include/libraries/gfopenmulticore.h`. One AmigaOS Exec on CPU0; secondary cores run 68k jobs, not Exec or chipset. `OMC_Submit`/`OMC_Wait`/`OMC_Check`/`OMC_Abort`, and range jobs through `OMC_SubmitRange`/`OMC_RunRange` **only** when `OMC_HAS_RANGE(base)` succeeds. `OMC_JobInit` and grants require `OMC_HAS_JOBCALLS(base)`. Jobs use `OMC_ANY` by default, or explicit target for qualified tests. `OMCF_STRICT`/`OMCF_PAGES` plus 16-byte cache-line-aligned write grants (`OMC_GRANTALIGN`); never grant Chip RAM. `OMC_AllocGrant` provides appropriate shared allocation. `OMCERR_TIMEOUT`, `OMCERR_FAULT`, `OMCERR_LOST`, `OMCERR_NOCORE` and `OMCERR_BUSY` require safe fallbacks. No-board path executes on CPU0. Library 0.6 also caches placement settings in ENV; library API version/revision gating remains mandatory.
* AkikoPlus: AmiCradle `AKIKOPLUS.md` and `app/machines/devices/cards/akikoplus/guest/akikoplus_dev.c`. Host-service methods include `media.decode/1` (through `akikoplus.device` ACMedia stream API for guest, falling to Mitchy on machines without AkikoPlus), `akiko.z/1` zlib compress/decompress, `akiko.fill/1`, `akiko.scale/1`, `akiko.convert/1`, `akiko.swap/1`, `akiko.c2p/1`. The media device's async stream read provides pulled S16BE stereo PCM; controls, completion and rate/PTS metadata need actual adapter handling. OPEN/PROBE require **process context**, never interrupt context. Availability and codec/stream format failures must return cleanly and choose a fallback.
* Existing audio/graphics: GFOpenGPU native SDL3, OpenAL.module with AHI Voices and SGI GLU have already passed isolated native 68040 guest tests; don't replace GPU operations with secondary-core CPU work unless measured faster. `DalsinAI/gfopendatatypes` offers `gfopensound.datatype` Ogg Vorbis/MP3/FLAC decoding and native 68k fallback for a machine without media board. `DalsinAI/openamigaaudiotools` documents streamed datatype reads. Oolite's existing `OOALSoundDecoder.m` assumes libvorbisfile; a real GFOS decoder adapter is needed to avoid a bundled duplicate libvorbis runtime.

## Ownership boundary — non-negotiable

CPU0 alone owns Exec, Universe, GameController, live ShipEntity/PlayerEntity/Entity objects, scripting `JSTask` state and the game owner message port. Existing GFOS Javascript->game dispatch `OOGFOSGameDispatchPumpFrame` is nonblocking and budgeted; secondary workers return **plain immutable results**, never Objective-C objects, pointers into live entities, Exec handles, GL state or JS contexts. CPU0 snapshots immutable aligned inputs, submits jobs, continues simulation/rendering where independent, validates generation/sequence numbers and applies the results at a deterministic frame boundary. Never invoke ObjC/JS/SDL/OpenAL from a secondary core. For game-world transforms, avoid changing order of gameplay events, random-number generation, AI decisions or physics integration without differential tests.

## Prioritized Oolite workloads

| Priority | Workload | Candidate engine seam | Execution | Fallback and proof |
| --- | --- | --- | --- | --- |
| P0 | Ogg Vorbis sound/music decode | `OOALSoundDecoder.m` into PCM buffer feeding original OpenAL sound path | AkikoPlus/ACMedia media decode; standalone GFOpenDatatypes when no board | Actual Ogg asset round-trip, 16-bit BE sample rate/channel/seek/end-of-stream tests, AHI audible output, no repeated libvorbis decoder in release |
| P0 | Compressed resource unpacking | resource loader/archive workers, not mutable ObjC parser | AkikoPlus `akiko.z/1` for supported zlib streams | CPU0 zlib fallback, byte-identical decompression/hash/CRC, corrupt input and no-board tests |
| P1 | Batch starfield / effects / geometry preprocessing | extract plain-C, immutable arrays from current `Universe`/render paths | `OMC_SubmitRange` for independent slices, `OMC_Submit` for larger jobs | Byte/numeric tolerance compare vs CPU0, bench frame time incl grants; fall back when below break-even |
| P1 | Texture preparation / scaling / format conversion | resource decode -> texture upload staging (not GL calls themselves) | AkikoPlus `akiko.scale`/`akiko.convert` where matching format; GFOpenGPU preferred for GPU-native work | Pixel parity, byte order, strides, size boundaries, test no-board + cross-endian |
| P1 | Bulk CPU-only spatial or visibility precomputation | copied positions/radii/IDs, output per-index flags, *not* Entity pointers | `OMC_RunRange` with disjoint write slices | CPU0 authoritative final ordering and culling, deterministic replay and correctness compare |
| P2 | Resource metadata parsing | isolated buffer-only C tokenizer/indexer where worthwhile | multicore or CPU0 background task, not ObjC parser execution on board core | fuzzed input, lifecycle/cancellation and measured gain |

Offload only when total wall-frame time is reduced after copy/grant/scheduling/completion overhead. The service implementations have measured call costs and CPU0 fallback: very small work should remain on CPU0. Prefer a bounded queue and persistent output slots, limit work per frame to avoid starving game task. Do not infer speed gains from standalone host microbenchmarks alone.

## Integration contract / suggested modules

1. Add a C-only `OOGFOSOffload` façade, with no AmigaOS types in Objective-C headers (avoids the Exec BOOL conflict already caught in `OOGFOSGameFrame.h`). Capability probing should distinguish available GFOpenMulticore jobcalls/ranges and AkikoPlus media/zlib, with a documented stable CPU0 fallback.
2. Define immutable job packet headers `{kind, version, generation, sequence, byteLength, status}` and separate aligned input/output memory grants. No pointers to Objective-C heap or Exec resources in packets. Strict range jobs own disjoint outputs; outputs become visible only after successful completion. If a job is lost/times out, don't reuse/free its granted buffers until abort/ownership is resolved.
3. Implement and qualify the sound adapter first: `akikoplus.device` decode into S16BE frames and metadata, feed canonical OpenAL buffers on CPU0, and use `gfopensound.datatype` fallback. Original `OOALSoundDecoder.m` should call the adapter with explicit lifetime and seek semantics; this is not satisfied by its current successful libvorbisfile compile.
4. Implement a representative pure-C range worker next: independent index-to-result arrays, deterministic CPU0 reference, strict memory grants and board/no-board parity. Test worker crash/timeout/busy/lost service, slow jobs, task shutdown and stale generation results. Expose the measured size threshold for actual offload in a configuration option, not hardcode it from guesses.
5. Integrate into Oolite's original update/event queue only after baseline full engine link and startup. Keep GPU rendering on owner task and validate gameplay determinism with offload enabled vs disabled.

## Required release gates

* Compile against canonical SDK rather than inventing stubs for multicore/AkikoPlus, build 68040 OS3.2.3 and target AROS separately.
* On dedicated AC090 guest: prove AkikoPlus present and absent routes; same Ogg test decodes to identical PCM or documented numeric equivalence; OpenAL/AHI plays it; seek/EOF and shutdown behave; invalid inputs fail without crashes.
* On multicore board and plain A1200: CPU0 and worker outputs equal; no Exec calls/Chip RAM/ObjC on workers; strict grants and endianness correct; lifetime safety after timeout/loss; race tests and instrumentation of task placement.
* Record frame-time p50/p95/p99, CPU0 time, queue depth, board occupancy and job overhead at representative scene sizes. Keep an offload kill switch and per-feature fallbacks.
* Source, tests, real guest logs and measured performance in GitHub before marking any offload as production-ready.

## Actual status

This document is an **integration decision and test plan**, not code implementing these offloads. The existing standalone original Oolite compile audit has no claimed AkikoPlus/GFOpenMulticore production adapter. No full playable Oolite engine or automatic speedup is yet proven.
