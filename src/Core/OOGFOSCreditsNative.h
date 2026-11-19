/* Oolite Objective-C-safe credit presentation API: no Exec BOOL headers
 * in a Foundation Objective-C translation unit. 68040 LONG is 32-bit
 * signed long. Bridge implementation in OOGFOSCredits.c. GPL-2.0+.
 */
#ifndef OOGFOS_CREDITS_NATIVE_H
#define OOGFOS_CREDITS_NATIVE_H
long OOGFOSCreditsFormat(unsigned long long tenths, int decimal, int symbol,
                        char *utf8, long capacity);
#endif
