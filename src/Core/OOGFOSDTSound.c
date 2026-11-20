/* Sounds through the datatypes on GFOS: see OOGFOSDTSound.h.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>
#include <intuition/classusr.h>
#include <utility/tagitem.h>
/* our own base, so nothing else in the link that names DataTypesBase clashes */
#define DataTypesBase OOGFOSDataTypesBase
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/intuition.h>
#include <clib/alib_protos.h>
#include "OOGFOSDTSound.h"

/* from gfopendatatypes Datatypes/include/datatypes/gfopenimage.h (47.2) */
#define OIA_Dummy      (TAG_USER + 0x0DA15000)
#define OIA_Reader     (OIA_Dummy + 7)
#define OIA_Channels   (OIA_Dummy + 10)
#define OIM_READSOUND  (TAG_USER + 0x0DA15903)
struct oiReadSound {
    ULONG MethodID;
    struct GadgetInfo *oirs_GInfo;
    WORD *oirs_Buffer;
    ULONG oirs_Frames;
};

struct Library *OOGFOSDataTypesBase;   /* opened at first use, kept */

struct OOGFOSDTSound {
    Object *o;
    char *path;
    int channels;
    long rate;
    unsigned long frames;
};

static int openLib(void)
{
    if (!DataTypesBase)
        DataTypesBase = OpenLibrary((CONST_STRPTR)"datatypes.library", 39);
    return DataTypesBase != NULL;
}

static int openObject(OOGFOSDTSound *s)
{
    ULONG ch = 0, rate = 0, len = 0;

    s->o = NewDTObject((APTR)s->path, DTA_GroupID, GID_SOUND, OIA_Reader, TRUE, TAG_DONE);
    if (!s->o)
        return 0;
    /* a datatype that answers OIA_Channels answers OIM_READSOUND */
    if (GetDTAttrs(s->o, OIA_Channels, (ULONG)&ch, TAG_DONE) != 1 || ch < 1 || ch > 2) {
        DisposeDTObject(s->o);
        s->o = NULL;
        return 0;
    }
    GetDTAttrs(s->o, SDTA_SamplesPerSec, (ULONG)&rate, SDTA_SampleLength, (ULONG)&len, TAG_DONE);
    if (!rate) {
        DisposeDTObject(s->o);
        s->o = NULL;
        return 0;
    }
    s->channels = (int)ch;
    s->rate = (long)rate;
    s->frames = len;
    return 1;
}

OOGFOSDTSound *OOGFOSDTSoundOpen(const char *path)
{
    OOGFOSDTSound *s;

    if (!path || !openLib())
        return NULL;
    s = calloc(1, sizeof *s);
    if (!s)
        return NULL;
    s->path = strdup(path);
    if (!s->path || !openObject(s)) {
        free(s->path);
        free(s);
        return NULL;
    }
    return s;
}

int OOGFOSDTSoundChannels(const OOGFOSDTSound *s) { return s->channels; }
long OOGFOSDTSoundRate(const OOGFOSDTSound *s) { return s->rate; }
unsigned long OOGFOSDTSoundFrames(const OOGFOSDTSound *s) { return s->frames; }

unsigned long OOGFOSDTSoundRead(OOGFOSDTSound *s, short *buffer, unsigned long maxFrames)
{
    struct oiReadSound m;

    if (!s->o || !buffer || !maxFrames)
        return 0;
    m.MethodID = OIM_READSOUND;
    m.oirs_GInfo = NULL;
    m.oirs_Buffer = (WORD *)buffer;
    m.oirs_Frames = maxFrames;
    return DoMethodA(s->o, (Msg)&m);
}

int OOGFOSDTSoundRewind(OOGFOSDTSound *s)
{
    if (s->o)
        DisposeDTObject(s->o);
    s->o = NULL;
    return openObject(s);
}

void OOGFOSDTSoundClose(OOGFOSDTSound *s)
{
    if (!s)
        return;
    if (s->o)
        DisposeDTObject(s->o);
    free(s->path);
    free(s);
}
