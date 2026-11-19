# Oolite Vector3D and Quaternion — GFOpenJavaScript v2 native qualification

Date: 19 November 2026.
Runtime: standalone GFOpenJavaScript v2 loader + real freshly built
JavaScriptCore v2 JavaScriptEngine (not SpiderMonkey or a host JS mock).
Guest: AmiCradle Team 1 - Oolite JSAPI RC1, AmigaOS 3.2.3, AC090 68040/FPU.

## Implemented native binding work

The GFOS host-class Vector3D adapter now implements the original
Vector3D.rotationTo(target[, angleLimit]) and Vector3D.rotateBy(quaternion)
operations in double-precision, including antiparallel, zero-vector
and constrained rotation cases.

A native Oolite Quaternion host class now provides:
- new Quaternion() identity and numeric/copy constructors
- w/x/y/z finite accessor properties, exact host wrapper read/create
- genuine registered JS_ClassPrototype, instanceof and constructor identity
- multiply, dot, normalize, conjugate, toArray, toString and toSource
- rotateX, rotateY, rotateZ
- vectorForward, vectorRight and vectorUp returning native Vector3D

These values are JavaScript-owned snapshots, not game-task Universe or
Entity pointers. The finalizer frees native wrapper data. All access is
on the owning JSTask and uses the canonical GFOpenSDK ABI.

## Native guest evidence

Source gate: tests/oolite_js_vector_quaternion_gate.c
Make target: js-vector-quaternion
AmigaOS 68040 binary SHA-256:
32f9f0d8371be5f0e807ac82858bfad392451425b2b1a1ed380ce2c983a337ee

Actual guest transcript:
tests/evidence/oolite-vector-quaternion-native-guest-20261119.log

PASS RC 0, exact result:
OOLITE_VECTOR_QUATERNION vector=0 quat=0 code=0 answer=42 read=0 invalid=1 recovered=42 status=PASS

Additional complete regression:
tests/evidence/oolite-vector-quaternion-regression-guest-20261119.log
- old Vector3D arithmetic/array/constructor regression: PASS RC 0
- strict v2 Vector3D native-prototype gate: PASS RC 0
- new Vector3D/Quaternion rotation semantics: PASS RC 0

The test checks identity, from/to array, finite invalid input rejection,
normalization, multiplication, conjugation, axis rotations, vector directions,
rotationTo, rotateBy, angular limit and exception recovery.

## What is not yet original Oolite parity

The original OOJSQuaternion.m and OOJSVector.m translation units are not
source-rebuilt without SpiderMonkey. Quaternion.rotate(axis,angle),
Quaternion.random, entity-as-vector/quat conversion, universe-specific
coordinate-system methods, random-vector static methods and all original
Oolite engine and OXP behaviour remain independent migration gates.
The original 242-object archive still has 101 unresolved historical JS_*
symbols across 53 objects. Do not describe this as a complete Oolite port.

Known numerical caveat: current special cases use normalized finite
double vectors/quaternions. Exhaustive comparison to upstream Oolite
quaternion_rotation_between on real Oolite game data remains required.
