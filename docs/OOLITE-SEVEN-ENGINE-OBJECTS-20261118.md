# Seven original Oolite engine modules compiled natively for AmigaOS 3.2.3 / 68040

18 November 2026, 15:00-15:10 UTC. Pinned OoliteProject/oolite HEAD `21978c37b2f11c8664adf3e2062efc1d0c875979`, with previously applied GFOS source patches in the upstream working tree. The source is not copied into this repository; `make oolite-core-seven-objects` compiles using canonical GFOpenGPU SDL3/OpenAL headers, real GNUstep Foundation and target SpiderMonkey 1.8.5 generated headers, GCC 16.2 `-O2 -m68040 -m68881 -noixemul`. No local Git commits.

The original four qualified modules `Universe.m` (opt-in frame hook), `MyOpenGLView.m`, `GameController.m` and `main.m` are extended by original `ResourceManager.m` (real upstream MiniZip include), `OOShipRegistry.m` and `OOSoundSource.m`. All seven built to actual m68k object files, with no fatal errors. Three new objects have native SHA-256:

- ResourceManager: `5d7c43ce10ff9c795d23e5feab2430998280efd898309994c48ee4869db1f56e`
- OOShipRegistry: `9d954ad78827b07269900bcd1675d0a4f6d16c307787a19e99fa579abfa65706`
- OOSoundSource: `d014f22ef429332cc17634043199355a39525a77eed71f806ac9728a8b4dc44c`

Actual GCC compiled C/ObjC, not a syntax-only approximation. This is a **compile gate**, not linked engine, app startup or gameplay. There are many unresolved original engine references to address.

Next real portability blocker: `OOLogHeader.m` refers to `OOCPUDescription` and `OOMemoryStatus` for logging, yet `OOCPUInfo.h` only exposes these on `OOLITE_WINDOWS || OOLITE_LINUX`. GFOS needs a proper native AmigaOS CPU/memory implementation and matching guarded declaration, not a misleading `OOLITE_LINUX=1` flag. Separately `OOOpenGL.m` uses GLU `gluErrorString`, requiring the canonical GLU integration. Existing Oolite `stb_image_write.h` compiler warnings also remain for review.

The accompanying SDL3 3.4.14 and OpenAL modules were built in private scratch; native rendercompare (18 scenes, RC0) and ALInfo with actual ACAHI Voices (RC0) were qualified in the **dedicated** AmigaChrome AC090 guest. The previous installed OpenAL module was backed up and restored byte-for-byte after testing; no shared stove or other team lab was modified. See `docs/OOLITE-SDL3-OPENAL-AC090-20261118.md`.
