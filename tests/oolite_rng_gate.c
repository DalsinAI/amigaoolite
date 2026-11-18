/* Actual Oolite world-generation RNG portability probe, upstream GPLv2+. */
#include <stdio.h>
#include <stdint.h>
#include "legacy_random.h"
int main(void) {
  unsigned a,b,c;
  ranrot_srand(0x12345678u);
  a=Ranrot(); b=Ranrot(); c=Ranrot();
  printf("RANROT %08x %08x %08x\n",a,b,c);
  clear_checksum();
  int c1 = munge_checksum(42);
  int c2 = munge_checksum(1234);
  printf("CHECKSUM %d %d\n",c1,c2);
  return 0;
}
