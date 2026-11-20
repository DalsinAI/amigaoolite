/*

OOALSoundDecoder-gfos.m

Oolite's sound decoder on GFOS: in place of upstream's OOALSoundDecoder.m,
which decodes Ogg Vorbis with libvorbisfile. Here the datatypes decode it
(OOGFOSDTSound.c: gfopensound.datatype, which sends Ogg Vorbis to the
services card, AkikoPlus, and decodes on the 68k only without one). So the
game links no libogg or libvorbis. The samples come 16-bit in the 68k's own
byte order, which is what OpenAL takes.

A sound inside an OXZ is unzipped to T: first, because a datatype reads a
file.

Oolite for GFOS: the interface is upstream's (OOALSoundDecoder.h,
Copyright (C) 2005-2013 Jens Ayton, MIT licence as upstream's file); this
implementation MIT, Copyright (c) 2026 AmiCradle Team.

*/

#import "OOALSoundDecoder.h"
#import "OOLogging.h"
#import "OOOpenALController.h"
#import "unzip.h"
#include "OOGFOSDTSound.h"
#include <stdio.h>
#include <stdlib.h>

@interface OOALSoundDTCodec : OOALSoundDecoder {
    OOGFOSDTSound* _sound;
    NSString* _name;
    NSString* _tempPath;   // an OXZ's sound, unzipped to T:
    BOOL _readStarted;
}
@end

@implementation OOALSoundDecoder

- (id)initWithPath:(NSString*)inPath
{
    [self release];
    self = nil;
    if ([[[inPath pathExtension] lowercaseString] isEqual:@"ogg"]) {
        self = [[OOALSoundDTCodec alloc] initWithPath:inPath];
    }
    return self;
}

+ (OOALSoundDecoder*)codecWithPath:(NSString*)inPath
{
    if ([[[inPath pathExtension] lowercaseString] isEqual:@"ogg"]) {
        return [[[OOALSoundDTCodec alloc] initWithPath:inPath] autorelease];
    }
    return nil;
}

- (size_t)streamToBuffer:(char*)ioBuffer { return 0; }

- (BOOL)readCreatingBuffer:(char**)outBuffer withFrameCount:(size_t*)outSize
{
    if (NULL != outBuffer)
        *outBuffer = NULL;
    if (NULL != outSize)
        *outSize = 0;
    return NO;
}

- (size_t)sizeAsBuffer { return 0; }
- (BOOL)isStereo { return NO; }
- (long)sampleRate { return 0; }
- (void)reset { }
- (NSString*)name { return @""; }

@end

/* The sound in an OXZ, written to T: for the datatype; nil when it can't be. */
static NSString* OOGFOSUnzipToTemp(NSString* zipFile, NSString* containedFile)
{
    static unsigned long sCount = 0;
    unzFile uf = unzOpen64([zipFile UTF8String]);
    NSString* result = nil;
    char buf[8192];
    int got;

    if (uf == NULL) {
        OOLog(kOOLogFileNotFound, @"Could not unzip OXZ at %@", zipFile);
        return nil;
    }
    if (unzLocateFile(uf, [containedFile UTF8String], 1) == UNZ_OK && unzOpenCurrentFile(uf) == UNZ_OK) {
        NSString* temp = [NSString stringWithFormat:@"T:oolite-sound-%lu.ogg", ++sCount];
        FILE* f = fopen([temp UTF8String], "wb");
        if (f != NULL) {
            BOOL ok = YES;
            while ((got = unzReadCurrentFile(uf, buf, sizeof buf)) > 0) {
                if (fwrite(buf, 1, (size_t)got, f) != (size_t)got) {
                    ok = NO;
                    break;
                }
            }
            fclose(f);
            if (ok && got == 0)
                result = temp;
            else
                remove([temp UTF8String]);
        }
        unzCloseCurrentFile(uf);
    } else {
        OOLog(kOOLogFileNotFound, @"Could not read %@ within OXZ at %@", containedFile, zipFile);
    }
    unzClose(uf);
    return result;
}

@implementation OOALSoundDTCodec

- (id)initWithPath:(NSString*)path
{
    if ((self = [super init])) {
        NSArray* components = [path pathComponents];
        NSUInteger i, cl = [components count];
        NSString* file = path;

        _name = [[path lastPathComponent] retain];
        for (i = 0; i < cl; i++) {
            if ([[[[components objectAtIndex:i] pathExtension] lowercaseString] isEqualToString:@"oxz"])
                break;
        }
        if (i < cl) {
            NSString* zipFile = [NSString pathWithComponents:[components subarrayWithRange:NSMakeRange(0, i + 1)]];
            NSString* contained = [NSString pathWithComponents:[components subarrayWithRange:NSMakeRange(i + 1, cl - (i + 1))]];
            _tempPath = [OOGFOSUnzipToTemp(zipFile, contained) retain];
            file = _tempPath;
        }
        _sound = file != nil ? OOGFOSDTSoundOpen([file UTF8String]) : NULL;
        if (_sound == NULL) {
            OOLog(kOOLogSoundLoadingError, @"The datatypes could not open sound \"%@\"", path);
            [self release];
            return nil;
        }
    }
    return self;
}

- (void)dealloc
{
    OOGFOSDTSoundClose(_sound);
    if (_tempPath != nil)
        remove([_tempPath UTF8String]);
    [_tempPath release];
    [_name release];
    [super dealloc];
}

- (BOOL)readCreatingBuffer:(char**)outBuffer withFrameCount:(size_t*)outSize
{
    size_t frameSize = [self isStereo] ? 4 : 2;
    size_t capacity = OOGFOSDTSoundFrames(_sound), done = 0;
    char* buffer;

    if (NULL != outBuffer)
        *outBuffer = NULL;
    if (NULL != outSize)
        *outSize = 0;
    if (NULL == outBuffer || NULL == outSize)
        return NO;
    if (capacity == 0)
        capacity = 65536;
    buffer = malloc(capacity * frameSize);
    if (buffer == NULL)
        return NO;
    for (;;) {
        unsigned long got;
        if (done == capacity) {   // the datatype said fewer frames than it has
            char* more = realloc(buffer, capacity * 2 * frameSize);
            if (more == NULL)
                break;
            buffer = more;
            capacity *= 2;
        }
        got = OOGFOSDTSoundRead(_sound, (short*)(buffer + done * frameSize), capacity - done);
        if (got == 0)
            break;
        done += got;
    }
    _readStarted = YES;
    if (done == 0) {
        free(buffer);
        return NO;
    }
    *outBuffer = buffer;
    *outSize = done * frameSize;
    return YES;
}

- (size_t)streamToBuffer:(char*)buffer
{
    size_t frameSize = [self isStereo] ? 4 : 2;
    size_t want = OOAL_STREAM_CHUNK_SIZE / frameSize, done = 0;

    _readStarted = YES;
    // the stream's own process decodes ahead, four refills' worth
    done = OOGFOSDTSoundStream(_sound, (short*)buffer, want, 4 * OOAL_STREAM_CHUNK_SIZE);
    return done * frameSize;
}

- (size_t)sizeAsBuffer
{
    return (size_t)OOGFOSDTSoundFrames(_sound) * ([self isStereo] ? 4 : 2);
}

- (BOOL)isStereo
{
    return OOGFOSDTSoundChannels(_sound) > 1;
}

- (long)sampleRate
{
    return OOGFOSDTSoundRate(_sound);
}

- (void)reset
{
    if (!_readStarted)
        return;
    if (!OOGFOSDTSoundRewind(_sound))
        OOLog(kOOLogSoundLoadingError, @"Could not start sound \"%@\" again", _name);
    _readStarted = NO;
}

- (NSString*)name
{
    return [[_name retain] autorelease];
}

- (NSString*)description
{
    return [NSString stringWithFormat:@"<%@ %p>{\"%@\", %ld Hz, %s}", [self className], self, _name, [self sampleRate],
                     [self isStereo] ? "stereo" : "mono"];
}

@end
