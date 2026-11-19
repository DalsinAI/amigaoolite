/* Oolite regex bridge: narrow C ABI without Exec/Objective-C BOOL collision.
 * JavaScriptCore RegExp semantics via canonical javascript.library.
 * GPL-2.0-or-later (bridge); does not embed SpiderMonkey JSAPI.
 */
#ifndef OOGFOS_REGEX_H
#define OOGFOS_REGEX_H
int OOGFOSRegExpOpen(void);
void OOGFOSRegExpClose(void);
/* Inputs are UTF-8; supported masks mirror original Oolite:
 * 0x01 ignoreCase, 0x04 multiline. 1=match, 0=no match/error. */
int OOGFOSRegExpMatch(const char *source,const char *pattern,
                      unsigned long flags);
#endif
