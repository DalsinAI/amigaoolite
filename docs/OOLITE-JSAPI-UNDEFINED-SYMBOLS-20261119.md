# Original Oolite 68040: unresolved SpiderMonkey JSAPI link symbols

- Date: 2026-11-19
- Team: AmiCradle Team 1 / SacredTrees
- Source: pinned upstream OoliteProject/oolite `21978c37b2f11c8664adf3e2062efc1d0c875979`
- Object corpus: 242 original 68040 translation-unit objects from isolated `oolite-rc1/evidence/rc1-title-20261119/build/oolite-complete-objects/`
- Tool: OpenAmigaGCC 16.2 m68k-amigaos-nm -u, normalized one leading target-symbol underscore
- Count: **101 distinct unresolved `JS_*` symbols**
- Meaning: binary-level linker references against the *historical* SpiderMonkey-facing objects, not an implementation, a complete ABI map, or evidence that a source-level shim is safe

This inventory is the concrete GFOS source-compatibility worklist. Class callbacks, rooting, runtime/context creation, exceptions, JS value/string/array operations, XDR and `JS_NewUCRegExpObjectNoStatics` all appear. The latter must be evaluated alongside GFOpenRegex, but a regular-expression library does not provide the JavaScript object/runtime semantics by itself.

The current production target is **javascript.library + GFOS adapter only**, with game-specific entity bindings/IPC still owned by Oolite. These objects must be migrated and rebuilt; simply supplying 101 names as stub symbols would falsely imply compatibility.

## Symbols

```text
JS_AddNamedValueRoot
JS_AddObjectRoot
JS_AddValueRoot
JS_CallFunction
JS_CallFunctionValue
JS_ClearPendingException
JS_ClearScope
JS_CompareStrings
JS_CompileUCFunction
JS_CompileUCScript
JS_ComputeThis
JS_ConvertStub
JS_DefineFunction
JS_DefineFunctions
JS_DefineObject
JS_DefineProperties
JS_DefineProperty
JS_DefinePropertyById
JS_DeleteProperty
JS_DestroyContext
JS_DestroyIdArray
JS_DestroyScript
JS_Enumerate
JS_EnumerateStub
JS_ExecuteScript
JS_FinalizeStub
JS_Finish
JS_FreezeObject
JS_GC
JS_GetArrayLength
JS_GetClass
JS_GetElement
JS_GetEmptyStringValue
JS_GetFunctionId
JS_GetFunctionObject
JS_GetInstancePrivate
JS_GetMethodById
JS_GetPendingException
JS_GetPrivate
JS_GetProperty
JS_GetPropertyById
JS_GetStringCharsAndLength
JS_GetVersion
JS_IdToValue
JS_Init
JS_InitClass
JS_InitStandardClasses
JS_InstanceOf
JS_InternString
JS_InternUCStringN
JS_IsArrayObject
JS_IsExceptionPending
JS_LookupElement
JS_LookupPropertyById
JS_MaybeGC
JS_NewArrayObject
JS_NewCompartmentAndGlobalObject
JS_NewContext
JS_NewNumberValue
JS_NewObject
JS_NewScriptObject
JS_NewStringCopyZ
JS_NewUCRegExpObjectNoStatics
JS_NewUCStringCopyN
JS_ObjectIsFunction
JS_PropertyStub
JS_RemoveObjectRoot
JS_RemoveValueRoot
JS_ReportError
JS_ReportPendingException
JS_ReportWarning
JS_ResolveStub
JS_SetContextCallback
JS_SetCStringsAreUTF8
JS_SetElement
JS_SetErrorReporter
JS_SetGlobalObject
JS_SetOperationCallback
JS_SetOptions
JS_SetPendingException
JS_SetPrivate
JS_SetProperty
JS_SetPropertyById
JS_SetVersion
JS_StrictPropertyStub
JS_StringEqualsAscii
JS_TriggerAllOperationCallbacks
JS_ValueToBoolean
JS_ValueToECMAUint32
JS_ValueToFunction
JS_ValueToId
JS_ValueToInt32
JS_ValueToNumber
JS_ValueToObject
JS_ValueToString
JS_VersionToString
JS_XDRDestroy
JS_XDRMemGetData
JS_XDRMemSetData
JS_XDRNewMem
JS_XDRScript
```

## Integration gates

1. Classify each symbol into exact / semantic adapter / unsupported, with concrete type/lifetime/calling convention requirements and test cases.
2. Adapt original `OOJavaScriptEngine` lifecycle and source bindings to the canonical GFOS `JSTask`/host-class model, retaining synchronous game-task semantics.
3. Build all 242 original Oolite units without `libjs_static.a`, link the executable, verify no embedded SpiderMonkey runtime.
4. Native AC090 guest full-engine startup, real Oolite title, ship/world script events, flight, saves and clean shutdown.
