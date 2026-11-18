# SpiderMonkey 1.8.5 port prerequisite: native 68040 ABI

18 November 2026. `tests/oolite_js_abi_68040_gate.c` runs as an actual native AmigaOS loadseg executable inside the dedicated AmigaChrome AC090 68040 guest (GCC 16.2 `-m68040 -m68881 -noixemul`). It checks target pointer/long/size_t and integer/double widths and runtime endianness, to avoid carrying the host-specific 64-bit architecture assumptions from Oolite's macOS SpiderMonkey 1.8.5 build recipe into the 68040 port.

```
OOLITE_JS_ABI_BEGIN
OOLITE_JS_ABI ptr=4 long=4 size_t=4 u64=8 double=8 long_double=12 big_endian=1 status=PASS
OOLITE_JS_ABI_RC 0
OOLITE_JS_ABI_END
```

Binary SHA256 `b32619e849b730c9501ac444e68b8d3f0c102de0629fc8be6a8c183caa98533c`. Guest stdout `tests/evidence/oolite-js-abi-68040-guest.log`.

This only verifies guest C ABI properties. It does NOT mean that NSPR, SpiderMonkey, JSAPI, Objective-C scripting bindings or Oolite's full engine have been built or run. The next target build must generate genuine m68k target 32-bit/big-endian SpiderMonkey headers/configuration, compile an interpreter-only library and pass a guest JSAPI test before the Oolite game engine can link.
