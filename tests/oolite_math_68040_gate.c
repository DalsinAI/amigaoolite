/* Native Oolite upstream OOQuaternion / OOMatrix trig portability qualification. */
#include "OOMaths.h"
#include <stdio.h>
#include <math.h>
static int nearf(OOScalar x, OOScalar y) { return fabsf(x-y)<0.002f; }
int main(void) {
  Quaternion q;
  quaternion_set_rotate_about_axis(&q,make_vector(0.0f,0.0f,1.0f),(OOScalar)M_PI_2);
  OOMatrix m=OOMatrixForRotationX((OOScalar)M_PI_2);
  OOMatrix z=OOMatrixForRotationX(0.0f);
  int ok=nearf(q.w,0.70710678f) && nearf(q.z,0.70710678f) &&
    nearf(q.x,0) && nearf(q.y,0) &&
    nearf(m.m[0][0],1) && nearf(m.m[1][1],0) &&
    nearf(z.m[0][0],1) && nearf(z.m[1][1],1) && nearf(z.m[2][2],1);
  printf("OOLITE_MATH_68040 quaternion_w=%.5f quaternion_z=%.5f matrix_diag_y=%.5f zero_diag_y=%.5f status=%s\n",(double)q.w,(double)q.z,(double)m.m[1][1],(double)z.m[1][1],ok?"PASS":"FAIL");fflush(stdout);
  return ok?0:2;
}
