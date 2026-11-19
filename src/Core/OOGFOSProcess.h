/* What GNUstep needs to know about a GFOS process before it starts. */
#ifndef OOGFOS_PROCESS_H
#define OOGFOS_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Makes argv[0] the absolute path of the program file (in place; the string
 * is static). Call before GSInitializeProcess(). Returns argv. */
char **OOGFOSAbsoluteArgv(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
