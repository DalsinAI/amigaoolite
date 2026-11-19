/* Oolite native credit rendering / optional world script localization.
 * The game keeps ownership of numbers; Javascript is invoked only when
 * an installed OXP/world script supplies formatCredits(). GPL-2.0+.
 */
#ifndef OOGFOS_CREDITS_H
#define OOGFOS_CREDITS_H
#include <exec/types.h>
#include <exec/libraries.h>
#include <libraries/javascript.h>
/* Must be paired around Oolite JSTask lifecycle: clearing prior to task end.
 * Caller owns task; this is a borrowed reference, not an additional owner. */
void OOGFOSCreditsBindTask(JSTask task, struct Library *library);
void OOGFOSCreditsUnbindTask(JSTask task);
/* Caller supplies a UTF-8 buffer; return JSERR_OK only on successful
 * conversion of the JS-supplied string. Preserve Oolite's native fallback
 * by leaving the output empty on exceptions / no registered task. */
LONG OOGFOSCreditsFormat(unsigned long long tenths, int decimal, int symbol,
                        char *utf8, LONG capacity);
#endif
