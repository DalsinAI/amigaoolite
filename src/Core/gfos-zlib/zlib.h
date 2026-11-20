/* The part of zlib's API Oolite's MiniZip uses, answered by z.library at run
 * time (GFOpenZlib 53.13 or later, or the Aminet z.library 53). The GFOS build
 * puts this directory ahead of any other zlib.h. z_stream is laid out as
 * z.library lays it out on the 68k (56 bytes).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OOGFOS_ZLIB_H
#define OOGFOS_ZLIB_H
#ifdef __cplusplus
extern "C" {
#endif

#define ZEXPORT
#define ZEXTERN extern
#define OF(args) args
#define ZLIB_VERSION "1.2.3 (z.library)"

typedef unsigned char Byte;
typedef Byte Bytef;
typedef char charf;
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef uLong uLongf;
typedef void *voidpf;
typedef void *voidp;
typedef long z_off_t;

#define Z_NO_FLUSH      0
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
#define Z_DEFAULT_COMPRESSION (-1)
#define Z_DEFLATED      8
#define MAX_WBITS       15

typedef voidpf (*alloc_func)(voidpf opaque, uInt items, uInt size);
typedef void (*free_func)(voidpf opaque, voidpf address);

typedef struct z_stream_s {
    const Bytef *next_in;
    uInt avail_in;
    uLong total_in;
    Bytef *next_out;
    uInt avail_out;
    uLong total_out;
    const char *msg;
    void *state;
    alloc_func zalloc;
    free_func zfree;
    voidpf opaque;
    int data_type;
    uLong adler;
    uLong reserved;
} z_stream;
typedef z_stream *z_streamp;

int inflateInit2(z_streamp strm, int windowBits);
int inflate(z_streamp strm, int flush);
int inflateEnd(z_streamp strm);
uLong crc32(uLong crc, const Bytef *buf, uInt len);

#ifdef __cplusplus
}
#endif
#endif
