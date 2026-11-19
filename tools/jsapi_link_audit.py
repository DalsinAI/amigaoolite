#!/usr/bin/env python3
"""Audit unresolved SpiderMonkey APIs from pinned original Oolite 68k objects."""
import argparse
from collections import defaultdict
import json
from pathlib import Path
import re
import subprocess
import sys

GROUPS = {
    "lifecycle": "Init Finish NewRuntime NewContext DestroyContext NewCompartmentAndGlobalObject SetGlobalObject ClearScope InitStandardClasses SetContextCallback SetVersion GetVersion VersionToString SetOptions SetCStringsAreUTF8",
    "gc_roots": "AddObjectRoot AddValueRoot AddNamedValueRoot RemoveObjectRoot RemoveValueRoot GC MaybeGC TriggerAllOperationCallbacks",
    "native_classes": "InitClass GetClass InstanceOf GetInstancePrivate GetPrivate SetPrivate NewObject DefineObject ConvertStub FinalizeStub EnumerateStub ResolveStub PropertyStub StrictPropertyStub FreezeObject",
    "properties_arrays": "GetProperty SetProperty SetPropertyById DeleteProperty DefineProperty DefineProperties DefinePropertyById GetPropertyById LookupPropertyById LookupElement GetElement SetElement NewArrayObject IsArrayObject GetArrayLength Enumerate DestroyIdArray IdToValue ValueToId GetMethodById",
    "functions_scripts": "CallFunction CallFunctionValue DefineFunction DefineFunctions GetFunctionId GetFunctionObject ObjectIsFunction ValueToFunction CompileUCScript ExecuteScript DestroyScript CompileUCFunction NewScriptObject ComputeThis",
    "values_strings": "ValueToBoolean ValueToNumber ValueToInt32 ValueToECMAUint32 ValueToObject ValueToString NewNumberValue NewStringCopyZ NewUCStringCopyN GetStringCharsAndLength InternString InternUCStringN CompareStrings StringEqualsAscii GetEmptyStringValue",
    "errors": "ClearPendingException GetPendingException SetPendingException IsExceptionPending ReportError ReportWarning ReportPendingException SetErrorReporter SetOperationCallback",
    "xdr_regex": "XDRNewMem XDRMemGetData XDRMemSetData XDRScript XDRDestroy NewUCRegExpObjectNoStatics",
}
GROUPS = {k: set("JS_" + s for s in v.split()) for k, v in GROUPS.items()}
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("objects", type=Path)
    ap.add_argument("--nm", default="m68k-amigaos-nm")
    ap.add_argument("--json-out", type=Path)
    ap.add_argument("--markdown-out", type=Path)
    ap.add_argument("--expected-unique", type=int, default=None)
    ap.add_argument("--max-unique", type=int, default=101)
    a = ap.parse_args()
    objs = sorted(a.objects.glob("*.o"))
    if not objs:
        ap.error("No original object files at supplied path")
    proc = subprocess.run([a.nm, "-u", *map(str, objs)], capture_output=True, text=True)
    if proc.returncode:
        ap.error("nm failure: " + proc.stderr[:300])
    per = defaultdict(set)
    current = None
    for line in proc.stdout.splitlines():
        if line.endswith(".o:"):
            current = Path(line[:-1]).name
        elif current and re.search(r"\bU\s+_?JS_[A-Za-z0-9_]+\s*$", line):
            per[current].add(line.split()[-1].lstrip("_"))
    allsym = set().union(*per.values())
    unknown = sorted(allsym - set().union(*GROUPS.values()))
    overlap = sorted(s for s in allsym if sum(s in v for v in GROUPS.values()) > 1)
    rec = {
        "objects": len(objs),
        "objects_using_spidermonkey": len(per),
        "unique_spidermonkey_symbols": len(allsym),
        "groups": {k: sorted(allsym & v) for k, v in GROUPS.items()},
        "unknown": unknown,
        "overlap": overlap,
        "per_object": {k: sorted(v) for k, v in sorted(per.items())},
    }
    md = [
        "# Original Oolite SpiderMonkey dependency audit", "",
        "Derived from unresolved external symbols in the pinned 68040 object corpus.",
        "No runtime is replaced or stubbed by this audit.", "",
        f"- Total original objects: **{len(objs)}**",
        f"- Objects depending on SpiderMonkey: **{len(per)}**",
        f"- Unique unresolved JS APIs: **{len(allsym)}**", "",
        "## By responsibility", "",
        "| Family | Distinct symbols |", "|---|---:|",
    ]
    md.extend(f"| {k} | {len(allsym & v)} |" for k, v in GROUPS.items())
    md += ["", "## By original translation unit", "",
           "| Object | Distinct JS APIs |", "|---|---:|"]
    md.extend(f"| {k} | {len(v)} |" for k, v in sorted(per.items(), key=lambda x: (-len(x[1]), x[0])))
    md += ["", "## Classification exceptions", "",
           "Unknown: " + (", ".join(unknown) or "none"),
           "Overlapping: " + (", ".join(overlap) or "none"), "",
           "## Next gate", "",
           "Rebuild original objects against the GFOS source adapter, removing all",
           "external JS_* references and libjs_static.a from the complete game link.",
           "Source-only compatibility is not binary SpiderMonkey compatibility.", ""]
    if a.json_out:
        a.json_out.parent.mkdir(parents=True, exist_ok=True)
        a.json_out.write_text(json.dumps(rec, indent=2, sort_keys=True)+"\n")
    if a.markdown_out:
        a.markdown_out.parent.mkdir(parents=True, exist_ok=True)
        a.markdown_out.write_text("\n".join(md))
    print(f"objects={len(objs)} js_objects={len(per)} js_symbols={len(allsym)} unknown={len(unknown)} overlap={len(overlap)}")
    return 0 if (not unknown and not overlap and len(allsym)<=a.max_unique
                 and (a.expected_unique is None or len(allsym)==a.expected_unique)) else 2
if __name__ == "__main__":
    sys.exit(main())
