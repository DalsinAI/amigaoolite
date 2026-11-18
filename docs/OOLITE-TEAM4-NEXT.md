# Oolite-specific GFOS / GFOpenUp / Team 4 integration gate

Oolite must use `gfopenmulticore.library` on Team 4's card, and the OpenUp Part / GFOpenSDK ecosystem. See the full contract in `docs/GFOS-OPENUP-MULTICORE-INTEGRATION.md`.

- CPU0 remains authoritative for Oolite Objective-C universe, Foundation `NSRunLoop`, Exec/DOS and GFOpenGPU/Audio calls.
- Extra cores: pure numeric/procedural/mesh kernels, Fast RAM grants only; use `OMC_Submit`/`OMC_RunRange`, `OMC_SetSignal` and feature/version guards. Preserve CPU0 fallback with deterministic output parity.
- No second threading runtime or custom accelerator protocol. Team 4's header and `gfopenservice.device` remain authoritative. Explicitly qualify true cores-board mode, not merely the standard AC090 emulator.
- OpenUp/SDK is the packaging and shared API route; Oolite doesn't directly ship an alternate GL/SDL/audio stack.

**Current proof:** Upstream Oolite `legacy_random.c` native guest ran and matched host; `OOIsNumberLiteral.m` native guest PASS 12/12 against GFOpenGNUstep; `NSNumberOOExtensions.m` native cross-build and guest PASS 4/4 (separate test). These are components, not a playable game or multicore validation.

## Next execution checklist

1. Retrieve and compile against canonical Team 4 multicore public header, with no local ABI redefinition, and run an `OMC_RunRange` probe in both CPU0 and a confirmed multicore-board guest; record `omj_Where`.
2. Refactor a deterministic planet/geometry numeric loop into a standalone worker with aligned Fast RAM grants. Compare hashes single CPU vs board core; measure speed and CPU0 time; integrate back into Oolite only after parity.
3. Complete GFOS signal-driven `NSRunLoop` and address Foundation compatibility; then tackle GFOpenGPU/SDL3/GLU, GFOpenAudio, scripting/NSPR, save/resource filesystem and playable run.
4. Create GFOpenUp Parts and SDK entries only after code, install paths, dependencies, licences and guest install/uninstall are fully proven.
