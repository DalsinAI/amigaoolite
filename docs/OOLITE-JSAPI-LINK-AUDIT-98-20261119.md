# Original Oolite SpiderMonkey dependency audit

Derived from unresolved external symbols in the pinned 68040 object corpus.
No runtime is replaced or stubbed by this audit.

- Total original objects: **242**
- Objects depending on SpiderMonkey: **51**
- Unique unresolved JS APIs: **98**

## By responsibility

| Family | Distinct symbols |
|---|---:|
| lifecycle | 14 |
| gc_roots | 8 |
| native_classes | 15 |
| properties_arrays | 20 |
| functions_scripts | 14 |
| values_strings | 15 |
| errors | 7 |
| xdr_regex | 5 |

## By original translation unit

| Object | Distinct JS APIs |
|---|---:|
| Core_Scripting_OOJavaScriptEngine_m.o | 48 |
| Core_Scripting_OOJSScript_m.o | 28 |
| Core_Scripting_OOJSMission_m.o | 20 |
| Core_Scripting_OOJSQuaternion_m.o | 19 |
| Core_Scripting_OOJSTimer_m.o | 19 |
| Core_Scripting_OOJSVector_m.o | 19 |
| Core_Scripting_OOJSSystemInfo_m.o | 17 |
| Core_Scripting_OOJSGlobal_m.o | 15 |
| Core_Scripting_OOJSPlayerShip_m.o | 14 |
| Core_Scripting_OOJSSystem_m.o | 14 |
| Core_Scripting_OOJSOolite_m.o | 13 |
| Core_Scripting_OOJSStation_m.o | 13 |
| Core_Scripting_OOJSShip_m.o | 12 |
| Core_Scripting_OOJSSoundSource_m.o | 12 |
| Core_Scripting_OOJSEquipmentInfo_m.o | 11 |
| Core_Scripting_OOJSPlayer_m.o | 11 |
| Core_Scripting_OOJSClock_m.o | 10 |
| Core_Scripting_OOJSFunction_m.o | 10 |
| Core_Scripting_OOJSVisualEffect_m.o | 10 |
| Core_Scripting_OOJSFlasher_m.o | 9 |
| Core_Scripting_OOJSManifest_m.o | 9 |
| Core_Scripting_OOJSShipGroup_m.o | 9 |
| Core_Scripting_OOJSSound_m.o | 9 |
| Core_Scripting_OOJSSun_m.o | 9 |
| Core_Scripting_OOJSDock_m.o | 8 |
| Core_Scripting_OOJSFrameCallbacks_m.o | 8 |
| Core_Scripting_OOJSEntity_m.o | 7 |
| Core_Scripting_OOJSMissionVariables_m.o | 7 |
| Core_Scripting_OOJSPlanet_m.o | 7 |
| Core_Scripting_OOJSWaypoint_m.o | 7 |
| Core_Scripting_OOJSWorldScripts_m.o | 7 |
| Core_Scripting_OOJSExhaustPlume_m.o | 6 |
| Core_Scripting_OOJSWormhole_m.o | 6 |
| Core_Scripting_OOJSSpecialFunctions_m.o | 5 |
| Core_Entities_ShipEntity_m.o | 4 |
| Core_Scripting_EntityOOJavaScriptExtensions_m.o | 4 |
| Core_Scripting_OOJSEngineTimeManagement_m.o | 4 |
| Core_Scripting_OOJSGuiScreenKeyDefinition_m.o | 4 |
| Core_Scripting_OOJSInterfaceDefinition_m.o | 4 |
| Core_Scripting_OOJSPopulatorDefinition_m.o | 4 |
| Core_Entities_PlayerEntity_m.o | 3 |
| Core_Scripting_OOJSFont_m.o | 3 |
| Core_Scripting_OOConstToJSString_m.o | 2 |
| Core_Universe_m.o | 2 |
| Core_Entities_DockEntity_m.o | 1 |
| Core_Entities_OOECMBlastEntity_m.o | 1 |
| Core_Entities_ShipEntityAI_m.o | 1 |
| Core_Entities_StationEntity_m.o | 1 |
| Core_GuiDisplayGen_m.o | 1 |
| Core_OOCommodities_m.o | 1 |
| Core_OOShipRegistry_m.o | 1 |

## Classification exceptions

Unknown: none
Overlapping: none

## Next gate

Rebuild original objects against the GFOS source adapter, removing all
external JS_* references and libjs_static.a from the complete game link.
Source-only compatibility is not binary SpiderMonkey compatibility.
