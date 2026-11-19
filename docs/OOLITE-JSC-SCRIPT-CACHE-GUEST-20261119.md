# Oolite JavaScriptCore bytecode cache, no SpiderMonkey XDR

Date: 19 November 2026, AmigaOS 3.2.3 AC090 68040.
Library: standalone GFOpenJavaScript v2 3,696-byte CI loader paired with
the newly compiled real v2 JavaScriptCore engine.

## Implementation

OOGFOSJSOpenCached() copies an optional bounded AmigaDOS cache path
into the OOGFOSJS owner state and passes JSA_CacheDir to JS_NewTask.
The prepared reset path now retains the same cache directory when it
creates a replacement JSTask. OOGFOSJSOpen() preserves the previous
default of disabled caching, so existing callers behave as before.
No SpiderMonkey XDR, fake JSAPI or parallel compiler has been embedded.

The optional directory is a trusted local application setting, not a
JS-supplied pathname. Script version/integrity and concurrent writer
hardening are still core library release concerns.

## Native evidence

A normal standalone JS shell file executed under CACHE=SYS:OoliteCache,
returned 42 twice with RC 0, and generated .jsc entries on the mounted
AmigaDOS volume. Modifying the source produced 99 with RC 0 and a
new cache entry, rather than replaying stale bytecode.

Evidence:
tests/evidence/oolite-gfjs-v2-cache-first-reuse-guest-20261119.log
and tests/evidence/oolite-gfjs-v2-cache-changed-guest-20261119.log.

The new Oolite-specific source gate (make js-cache) passed in the same
native AC090 guest across OOGFOSJSReset(), returning:
first=42, repeat=42, modified=99, reset=1, rc=0,0,0, status=PASS.
Two .jsc files were observed in SYS:OoliteCache/PortTest.
Binary SHA-256:
377a5961aa3ba1094176c72dcdc7c946169e3b9a204719662b1409e3a26eb0b4.
Transcript:
tests/evidence/oolite-gfjs-v2-cached-adapter-guest-20261119.log.

Four pre-existing native adapter test gates were also rebuilt against the
enlarged OOGFOSJS owner state and passed in the guest: values conversion,
unconditional reset, host-class reset/finalizer, and prepared reset/rollback.
All RC 0; transcript:
tests/evidence/oolite-js-adapter-reset-regression-v2-guest-20261119.log.

## Scope

This enables an Oolite-specific persistent source cache for callers that
explicitly use OOGFOSJSOpenCached. It does not change the startup path
of the original pinned OOJavaScriptEngine.m (still SpiderMonkey-facing).
Concurrent cache integrity, OXP cache versioning and original Oolite source
migration remain release gates.
