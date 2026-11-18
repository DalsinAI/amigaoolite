# Oolite Universe.m real cross-compilation audit and Objective-C ABI repair

18 November 2026 (14:46-14:49 UTC). `DalsinAI/amigaoolite` working against pinned upstream `OoliteProject/oolite@21978c37b2f11c8664adf3e2062efc1d0c875979`. The original `src/Core/Universe.m` was copied to isolated scratch, patched using our previously qualified `patches/oolite/0001-gfos-universe-frame-pump.patch` (applies cleanly), and then **actually subjected to GCC 16.2 m68k AmigaOS Objective-C syntax checks**. No vendor checkout was edited.

## Real dependency progression

1. Adding upstream `src/Core/Entities`, `Materials`, `Scripting`, `Debug` and `Tables` include folders resolved the initial `OOEntityWithDrawable.h` not found.
2. Supplying **real** original 1.8.5 `jsapi.h` and corresponding generated cross-target `js-config.h` from the existing SpiderMonkey scratch build advanced the compile beyond `OOJSPropID.h`. This is a header resolution check, **not** proof of a working complete SpiderMonkey runtime/link.
3. The original `OOSound.h` selects OpenAL by default and requires `AL/al.h`, which is not available in the inspected target include paths. For **diagnostic syntax compilation only**, `-DOOLITE_OPENAL=0` bypasses that old audio backend. Upstream itself warns that no sound implementation is then selected; this does not add audio functionality.
4. At that point the new frame hook caused a **real Objective-C ABI header collision**: `exec/types.h` defines `BOOL` as 16-bit while `objc/objc.h` defines it as an 8-bit `unsigned char`. The initial `OOGFOSGameFrame.h` included `OOGFOSGameDispatch.h`, dragging `exec/types.h` into Objective-C `Universe.m` after the ObjC definition. **Fixed** by using an opaque `struct OOGFOSGameDispatch` and ABI-compatible 32-bit `unsigned int` / `int` in the public frame-hook header, with all AmigaOS Exec types remaining in `OOGFOSGameFrame.c`. Verified the exact target's `ULONG` is `unsigned int`, not `unsigned long` (the latter produces conflicting prototypes in the cross compiler).
5. After this repair the **next real compile blocker** is upstream `MyOpenGLView.h` requiring `SDL3/SDL_video.h`. We did not invent a header or mislabel this as compiling the full class. A genuine target SDL3/GFOpenGPU integration is still required, along with later dependency diagnostics.

## Evidence

- New compile-only `tests/oolite_universe_frame_header_gate.m` imports **real GNUstep Foundation** and the frame hook header, asserts Objective-C `sizeof(BOOL)==1`, native 32-bit int ABI and exact function pointer signatures; GCC 16.2 m68k Objective-C compilation succeeded. Due to pre-existing GNUstep `defined` macro expansion warnings, this focused gate uses `-Wno-expansion-to-defined` while retaining `-Wall -Wextra -Werror`.
- `src/Core/OOGFOSGameFrame.c` separately compiles at `-Wall -Wextra -Werror`, and the original game-frame hook guest executable links with the new header without altering its SHA-256 `602fbe7233526801e432b0d3b38e8cafc0325f7ab27013b636fea1ad4c58b02e`.
- Full **native AmigaChrome AC090 68040** guest re-run after the ABI header fix returned 0:

```
OOLITE_JS_FRAME_HOOK bound=1 post=0 completed=1 requests=6 writes=3 wrong_task=0 on_owner=0 sum=90 idle=2 empty_budget=0 zero_budget=-1 processed=7 installed=1 uninstalled=1 idle_frame=0 foreign_frame=-1 foreign_unbind=0 status=PASS
OOLITE_OBJC_FRAME_ABI_RC 0
```

Guest stdout is `tests/evidence/oolite-universe-frame-objc-abi-guest.log`.

## Next engineering gate

Find/build the **canonical GFOS SDL3 video header and native GPU API**, or create an explicit `OOView` adapter against the canonical GPU library; do not use fabricated SDL3/OpenAL declarations. Then repeat actual `Universe.m` target compile to reveal the next dependencies, and only after those resolve enable `OOLITE_GFOS_GAME_DISPATCH` in an integrated engine target. Existing script worker shutdown/cancellation caveats remain. **Not a full Universe.m compile, link or playable Oolite.**
