# Native OXZ manifest minimum schema qualification

18 November 2026, native AmigaChrome AC090 68040 guest. Pinned Oolite commit `21978c37b2f11c8664adf3e2062efc1d0c875979`.

The upstream `ResourceManager +validateManifest:forOXP:` checks that `manifest.plist` contains `identifier`, `version`, `required_oolite_version` and `title`, then separately checks Oolite version compatibility and duplicate identifiers. `tests/oolite_oxz_manifest_schema_gate.m` uses upstream `OOManifestProperties.h` constants, Oolite's pinned MiniZip reader and GFOpenGNUstep 1.31.1 property-list serialization to verify the required four keys are nonempty NSString values. This is a standalone qualification harness; it is NOT the original full ResourceManager validation and does NOT yet check version compatibility, dependency/conflict policy or duplicate installed identifiers.

Two deliberately controlled compressed fixtures are `tests/fixtures/oolite-oxz-schema-valid.oxz` (four fields) and `tests/fixtures/oolite-oxz-schema-missing-title.oxz` (missing title). Stage as `SYS:Oolite/schema-valid.oxz` and `SYS:Oolite/schema-invalid.oxz` respectively.

Actual guest output:

```
OOLITE_OXZ_SCHEMA_BEGIN
OOLITE_OXZ_SCHEMA good_parsed=1 good_accepted=1 bad_parsed=1 missing_title_rejected=1 status=PASS
OOLITE_OXZ_SCHEMA_RC 0
OOLITE_OXZ_SCHEMA_END
```

Native GCC 16.2 68040 executable SHA-256 `b83993ef0403a0ab74e14579fe74367ee580d518dcb587fc4943576dbd905d1d`. Exact stdout `tests/evidence/oolite-oxz-schema-guest.log`.

Full OOOXZManager still requires game-engine GUI/SpiderMonkey integration and has not linked. Automatic arbitrary OXZ installation remains disabled.
