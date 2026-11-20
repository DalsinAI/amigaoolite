/*

OOPNGTextureLoader.m, Oolite for GFOS: in place of upstream's, which decodes
with libpng. A texture is read by the picture datatypes (OOGFOSDTPicture.c:
gfopenpng.datatype decodes on the services card, AkikoPlus, when there is one,
else on this CPU), so the game links no libpng and no zlib for its textures.

A texture inside an OXZ is written to T: first, because a datatype reads a
file.

Upstream's class, Copyright (C) 2004-2013 Giles C Williams and contributors,
GPL 2 or later as upstream's file; this implementation 2026 AmiCradle Team.

*/

#import "OOPNGTextureLoader.h"
#import "NSDataOOExtensions.h"
#import "OOLogging.h"
#include "OOGFOSDTPicture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

@implementation OOPNGTextureLoader

- (void)loadTexture
{
    unsigned long w = 0, h = 0;
    int alpha = 0;
    unsigned char* argb = NULL;
    NSString* temp = nil;
    FILE* f = fopen([_path fileSystemRepresentation], "rb");

    if (f != NULL) {                     // a plain file: the datatype reads it
        fclose(f);
        argb = OOGFOSDTPictureLoad([_path fileSystemRepresentation], &w, &h, &alpha);
    } else {                             // inside an OXZ: to T: first
        static unsigned long sCount = 0;
        NSData* data = [NSData oo_dataWithOXZFile:_path];
        if (data != nil) {
            temp = [NSString stringWithFormat:@"T:oolite-texture-%lu.png", ++sCount];
            if ([data writeToFile:temp atomically:NO])
                argb = OOGFOSDTPictureLoad([temp fileSystemRepresentation], &w, &h, &alpha);
            remove([temp fileSystemRepresentation]);
        }
    }
    if (argb == NULL) {
        OOLog(@"texture.load.png.failed", @"The datatypes could not read the picture %@", _path);
        return;
    }

    size_t i, n = (size_t)w * h;
    BOOL grey = !alpha;
    for (i = 0; grey && i < n; i++) {
        const unsigned char* p = argb + i * 4;
        if (p[1] != p[2] || p[2] != p[3])
            grey = NO;
    }
    if (grey) {                          // one byte a pixel, as libpng gave greyscale files
        unsigned char* g = (unsigned char*)argb;
        for (i = 0; i < n; i++)
            g[i] = argb[i * 4 + 1];
        _data = realloc(argb, n);
        if (_data == NULL)
            _data = argb;
        _format = kOOTextureDataGrayscale;
        _rowBytes = w;
    } else {
        /* kOOTextureDataRGBA on a big-endian CPU is A, B, G, R in memory (upstream's
           libpng settings: bgr, alpha swapped, filler before): swap R and B. */
        for (i = 0; i < n; i++) {
            unsigned char* p = argb + i * 4;
            unsigned char r = p[1];
            p[1] = p[3];
            p[3] = r;
        }
        _data = argb;
        _format = kOOTextureDataRGBA;
        _rowBytes = w * 4;
    }
    _width = (uint32_t)w;
    _height = (uint32_t)h;
}

@end
