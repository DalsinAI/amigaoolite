# Oolite original credits: game retains native arithmetic, JS formats text

19 November 2026 | DalsinAI/amigaoolite, Team 1.

## Original dependency

Original Core/OOStringParsing.m owns native credit values
(OOCreditsQuantity) but asks JavaScript global formatCredits(amount,
includeDecimal,includeSymbol) to display an optional localized string.
It uses JS_GetPendingException, JS_ClearPendingException,
JS_GetMethodById, JS_NewNumberValue, JS_CallFunctionValue and
JS_SetPendingException. Two APIs (JS_GetPendingException and
JS_SetPendingException) are unique to this unit in the original corpus.

## Implementation

The entire original OOStringParsing.m is copied to
src/Core/original-replacements/OOStringParsing.m with exactly the credits
function source ported: native arithmetic and native fallback remain.
A small OOGFOSCredits native C bridge calls an OXP/world script's optional
formatCredits via JS_Post, JS_Call, JS_NewDouble and JS_ToString.
It returns no usable output on a JS exception; original Oolite's
integer-credit fallback still applies.

Objective-C code imports OOGFOSCreditsNative.h, which deliberately does
not import Exec's conflicting BOOL typedef into a Foundation source.

The main scripting lifecycle must call OOGFOSCreditsBindTask when the
canonical Oolite JSTask becomes active and OOGFOSCreditsUnbindTask
before ending/resetting the task; without it, native fallback applies.

## Genuine native result

Actual GCC16.2 m68k 68040 bridge/test built with GFOpenSDK v2 and
ran on AmiCradle AC090 AmigaOS 3.2.3, after restoring the exact
known-good v2 JavaScript engine pair:

    OOLITE_NATIVE_CREDITS before=3 noMethod=5 formatted=0,0 exception=1 recovery=0 after=3 status=PASS
    OOLITE_NATIVE_CREDITS_RC 0

The test proves native input -> JavaScript formatCredits -> UTF-8 output
including Unicode, boolean display flags, correct numeric formatting,
missing handler fallback, JS exception handling and recovery; after
UnbindTask the bridge refuses a stale task handle.

Evidence: tests/evidence/oolite-original-credits-gfos-v2-native-20261119.log

## Objective-C integration BLOCKED, not qualified

Attempting to compile the entire original ObjC OOStringParsing.m source
currently fails because the previously pinned GFOpenGPU SDL3/OpenAL
and target JSAPI build headers are no longer mounted in the old
GFOpenRTG build-kit paths. Do not substitute fabricated SDL3/GL headers.
Makefile target js-original-credits-object explicitly checks authentic
headers, and jsapi-credits-rebuilt-audit will measure the effect ONLY when
the actual original translation unit has built.

**No genuine 242-object re-audit decrease from this unit yet.**
The verified original corpus remains 100 unresolved SpiderMonkey APIs
across 52 original object positions. This source-port change is a
functional native bridge plus unqualified original ObjC integration,
not a playable Oolite build.

Restore/reconstruct the original pinned genuine cross-build kit and then
compile/link Core_OOStringParsing_m.o, verify its ABI and rerun the audit.
