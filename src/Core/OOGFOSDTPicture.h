/* Pictures through the datatypes on GFOS: a PNG (or any picture a datatype
 * reads) opened as a picture datatype object and its pixels read back with
 * PDTM_READPIXELARRAY as 32-bit ARGB. gfopenpng.datatype decodes on the
 * services card (AkikoPlus) when there is one, else on this CPU, with no
 * libpng or zlib. Plain C, so the Objective-C side never sees the Amiga
 * headers. MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OOGFOS_DTPICTURE_H
#define OOGFOS_DTPICTURE_H

/* The picture at path, as width * height pixels of 0xAARRGGBB in memory
 * order A, R, G, B, from malloc() (the caller frees it). hasAlpha says
 * whether the file had alpha (pixels are 0xFF opaque when not). NULL when
 * no datatype reads it. */
unsigned char *OOGFOSDTPictureLoad(const char *path, unsigned long *width, unsigned long *height, int *hasAlpha);

#endif
