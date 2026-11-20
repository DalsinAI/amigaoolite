/* zlib's calls on z.library (zlib.h beside this), through its jump table in
 * the Aminet library's order and registers (its z_lib.sfd), which GFOpenZlib
 * follows from 53.13: -60 Inflate a0,d0, -66 InflateEnd a0,
 * -102 InflateInit2 a0,d0, -138 CRC32 d0,a0,d1. As amigaopenenroth's
 * amiga/zlibrary and gfopendatatypes' common/zlibrary.c.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <stdlib.h>

#include "zlib.h"

static const char noZlib[] = "z.library 53 is needed in LIBS: (GFOpenZlib, or the Aminet z.library)";

static struct Library *ZBase;
static int tried;

static void closeZlib(void)
{
    if (ZBase)
        CloseLibrary(ZBase);
    ZBase = NULL;
}

static int isGfopenzlib(const char *id)
{
    const char *w = "GFOpenZlib";
    for (; id && *id; id++) {
        int i = 0;
        while (w[i] && id[i] == w[i])
            i++;
        if (!w[i])
            return 1;
    }
    return 0;
}

static struct Library *zl(void)
{
    if (!tried) {
        struct Library *b;
        tried = 1;
        b = OpenLibrary((CONST_STRPTR)"z.library", 53);
        /* GFOpenZlib 53.12 does not follow the Aminet library's table */
        if (b && b->lib_Version == 53 && b->lib_Revision == 12 && isGfopenzlib((const char *)b->lib_IdString)) {
            CloseLibrary(b);
            b = NULL;
        }
        ZBase = b;
        if (ZBase)
            atexit(closeZlib);
    }
    return ZBase;
}

static LONG call2(LONG lvo, z_streamp s, LONG arg)
{
    register struct Library *a6 __asm("a6") = ZBase;
    register ULONG d0 __asm("d0") = (ULONG)arg;
    register ULONG d1 __asm("d1") = 0;
    register ULONG a0 __asm("a0") = (ULONG)s;
    register ULONG a1 __asm("a1") = 0;
    switch (lvo) {
    case -60: __asm volatile("jsr %%a6@(-60:W)" : "+r"(d0), "+r"(d1), "+r"(a0), "+r"(a1) : "r"(a6) : "cc", "memory"); break;
    case -66: __asm volatile("jsr %%a6@(-66:W)" : "+r"(d0), "+r"(d1), "+r"(a0), "+r"(a1) : "r"(a6) : "cc", "memory"); break;
    case -102: __asm volatile("jsr %%a6@(-102:W)" : "+r"(d0), "+r"(d1), "+r"(a0), "+r"(a1) : "r"(a6) : "cc", "memory"); break;
    default: return Z_STREAM_ERROR;
    }
    return (LONG)d0;
}

int inflateInit2(z_streamp s, int windowBits)
{
    s->zalloc = 0;      /* z.library allocates for itself */
    s->zfree = 0;
    s->opaque = 0;
    if (!zl()) {
        s->msg = noZlib;
        return Z_VERSION_ERROR;
    }
    return (int)call2(-102, s, windowBits);
}

int inflate(z_streamp s, int flush)
{
    return ZBase ? (int)call2(-60, s, flush) : Z_STREAM_ERROR;
}

int inflateEnd(z_streamp s)
{
    return ZBase ? (int)call2(-66, s, 0) : Z_STREAM_ERROR;
}

uLong crc32(uLong crc, const Bytef *buf, uInt len)
{
    if (!buf)
        return 0;       /* zlib: crc32(0, NULL, 0) is the start value */
    if (!zl())
        return 0;
    {
        register struct Library *a6 __asm("a6") = ZBase;
        register ULONG d0 __asm("d0") = crc;
        register ULONG d1 __asm("d1") = len;
        register ULONG a0 __asm("a0") = (ULONG)buf;
        register ULONG a1 __asm("a1") = 0;
        __asm volatile("jsr %%a6@(-138:W)" : "+r"(d0), "+r"(d1), "+r"(a0), "+r"(a1) : "r"(a6) : "cc", "memory");
        return d0;
    }
}
