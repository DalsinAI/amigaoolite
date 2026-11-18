# Native 68040 OXZ archive preflight — verified

18 November 2026; pinned upstream Oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

The actual upstream `OOOXZManager.m` extraction method is extended by `patches/0007-gfos-oxz-archive-preflight.patch`, applied after `0004` and `0006`. Before creating its extraction directory, the native GFOS branch invokes `OOGFOSPreflightOXZ` from `src/Core/OOOXZArchivePreflight.c`. The adapter uses the existing pinned Oolite MiniZip and `OOOXZPathSafety.h` (no second archive manager). The original non-GFOS path remains unchanged.

Preflight enumerates all archive entries before extraction. It rejects malformed file entries; non-relative, traversal, Amiga volume or control-character names; case-insensitive duplicate names; Unix symlinks; archives with more than 8,192 members; and declared total decompressed bytes over 256 MiB. Capacity checks avoid unsigned overflow. This is an explicit first-stage safety policy; real-world OXZ compatibility may require revisiting limits.

`tests/oolite_oxz_preflight_gate.c` ran in isolated AmigaChrome AC090 68040 guest using actual Oolite MiniZip and GFOS stdio. It accepted the controlled manifest fixture and rejected nonexistent, traversal (`../escape`), and case-colliding duplicate entry archives. The two deliberate bad archives are committed under `tests/fixtures/`.

```
OOLITE_OXZ_PREFLIGHT_BEGIN
OOLITE_OXZ_PREFLIGHT valid=1 entries=1 missing_rejected=1 escape_rejected=1 duplicate_rejected=1 status=PASS
OOLITE_OXZ_PREFLIGHT_RC 0
OOLITE_OXZ_PREFLIGHT_END
```

Final tested binary SHA-256: `503a44d01172cb995d9af56ba0c6a95c8c9cd316b3adc627203e0548b1869111`. Source, patch and guest stdout are preserved. Patch replay against the previously patched pinned upstream source passed.

## Important boundaries

The **full upstream OOOXZManager has NOT been built, linked, or executed**. The preflight function and test binary were independently compiled and executed; the patched extraction entrypoint has source-level replay validation only. TOCTOU between preflight and extraction, symlink targets through host filesystems, archive bombs/compression limits, malformed ZIP fuzzing, atomic extraction, rollback, package schema, actual installation and network downloads require additional work. Do not enable arbitrary OXZ installation yet.
