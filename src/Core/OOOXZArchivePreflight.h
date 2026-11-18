#ifndef OOLITE_GFOS_OXZ_PREFLIGHT_H
#define OOLITE_GFOS_OXZ_PREFLIGHT_H
/* Validate an OXZ archive before permitting any extraction. 0 = rejected. */
int OOGFOSPreflightOXZ(const char *path, unsigned *entries);
#endif
