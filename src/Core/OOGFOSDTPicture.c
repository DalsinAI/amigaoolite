/* Pictures through the datatypes on GFOS: see OOGFOSDTPicture.h.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdlib.h>
#include <exec/types.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <intuition/classusr.h>
#include <utility/tagitem.h>
/* our own base, so nothing else in the link that names DataTypesBase clashes
 * (the same one OOGFOSDTSound.c opens) */
#define DataTypesBase OOGFOSDataTypesBase
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>
#include "OOGFOSDTPicture.h"

extern struct Library *OOGFOSDataTypesBase;   /* OOGFOSDTSound.c */

#ifndef PDTA_AlphaChannel
#define PDTA_AlphaChannel (DTA_Dummy + 256)
#endif

unsigned char *OOGFOSDTPictureLoad(const char *path, unsigned long *width, unsigned long *height, int *hasAlpha)
{
    Object *o;
    struct BitMapHeader *bmh = NULL;
    ULONG alpha = 0, w, h;
    unsigned char *pixels = NULL;

    if (!DataTypesBase)
        DataTypesBase = OpenLibrary((CONST_STRPTR)"datatypes.library", 43);
    if (!DataTypesBase || !path)
        return NULL;
    o = NewDTObject((APTR)path, DTA_GroupID, GID_PICTURE, PDTA_DestMode, PMODE_V43, PDTA_Remap, FALSE, TAG_DONE);
    if (!o)
        return NULL;
    if (GetDTAttrs(o, PDTA_BitMapHeader, (ULONG)&bmh, TAG_DONE) != 1 || !bmh || !bmh->bmh_Width || !bmh->bmh_Height) {
        DisposeDTObject(o);
        return NULL;
    }
    w = bmh->bmh_Width;
    h = bmh->bmh_Height;
    GetDTAttrs(o, PDTA_AlphaChannel, (ULONG)&alpha, TAG_DONE);
    pixels = malloc((size_t)w * h * 4);
    /* DoMethodA, not DoDTMethodA: the second longword is the pixel array */
    if (pixels && !DoMethod(o, PDTM_READPIXELARRAY, (ULONG)pixels, PBPAFMT_ARGB, w * 4, 0, 0, w, h)) {
        free(pixels);
        pixels = NULL;
    }
    DisposeDTObject(o);
    if (pixels) {
        *width = w;
        *height = h;
        *hasAlpha = alpha ? 1 : 0;
        if (!alpha) {                       /* some datatypes leave the byte 0 */
            unsigned long i, n = w * h;
            for (i = 0; i < n; i++)
                pixels[i * 4] = 0xFF;
        }
    }
    return pixels;
}
