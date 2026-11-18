/* GFOS-only path validation for Oolite OXZ archive extraction.
 * Reject AmigaDOS volume escapes, absolute paths, dot segments, and control bytes.
 * The length is the archive's original filename length (not strlen of a truncated buffer).
 * Copyright (c) 2026 AmiCradle contributors. GPL-2.0-or-later.
 */
#ifndef OOLITE_GFOS_OXZ_PATH_SAFETY_H
#define OOLITE_GFOS_OXZ_PATH_SAFETY_H
#include <stddef.h>
static int OOGFOSSafeOXZEntry(const char *p, size_t n)
{
    size_t i, start = 0;
    if (p == NULL || n == 0 || n >= 512 || p[0] == '/') return 0;
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)p[i];
        if (c == 0 || c < 32 || c == 127 || c == ':' || c == '\\') return 0;
        if (c == '/') {
            size_t len = i - start;
            if (len == 0 || (len == 1 && p[start] == '.') ||
                (len == 2 && p[start] == '.' && p[start+1] == '.')) return 0;
            start = i + 1;
        }
    }
    if (start == n) return 1; /* directory, one final slash */
    if ((n - start == 1 && p[start] == '.') ||
        (n - start == 2 && p[start] == '.' && p[start+1] == '.')) return 0;
    return 1;
}
#endif
