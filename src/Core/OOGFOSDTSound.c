/* Sounds through the datatypes on GFOS: see OOGFOSDTSound.h.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>
#include <intuition/classusr.h>
#include <utility/tagitem.h>
/* our own base, so nothing else in the link that names DataTypesBase clashes */
#define DataTypesBase OOGFOSDataTypesBase
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/dos.h>
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

struct Stream {
    struct SignalSemaphore lock;
    UBYTE *ring;                 /* size bytes, frames written at head, read at tail */
    ULONG size, frameBytes;
    volatile ULONG head, tail;   /* bytes written and read, counted up */
    volatile BOOL eof, quit, failed, done;
    struct Task *reader;         /* the game's task, and its signal */
    BYTE readerSig;
    struct Task *worker;
    char *path;
};

struct OOGFOSDTSound {
    Object *o;
    char *path;
    int channels;
    long rate;
    unsigned long frames;
    struct Stream *stream;
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

/* frames decoded at a time by the stream's process: about 0.1 s at 44.1 kHz */
#define STREAM_PIECE 4096UL

/* The stream's process: its own datatype object, decoding ahead into the ring
 * whenever a piece fits, waiting on SIGBREAKF_CTRL_F when it doesn't. */
static void streamWorker(void)
{
    struct Stream *st = (struct Stream *)FindTask(NULL)->tc_UserData;
    Object *o = NewDTObject((APTR)st->path, DTA_GroupID, GID_SOUND, OIA_Reader, TRUE, TAG_DONE);
    struct oiReadSound m;

    if (!o)
        st->failed = TRUE;
    while (o && !st->quit) {
        ULONG used, at, room;
        ObtainSemaphore(&st->lock);
        used = st->head - st->tail;
        at = st->head % st->size;
        ReleaseSemaphore(&st->lock);
        room = st->size - used;
        if (st->size - at < room)
            room = st->size - at;               /* up to the ring's end, the rest next time */
        if (st->eof || room < st->frameBytes * 256) {
            Wait(SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_C);
            continue;
        }
        m.MethodID = OIM_READSOUND;
        m.oirs_GInfo = NULL;
        m.oirs_Buffer = (WORD *)(st->ring + at);
        m.oirs_Frames = room / st->frameBytes;
        if (m.oirs_Frames > STREAM_PIECE)
            m.oirs_Frames = STREAM_PIECE;
        m.oirs_Frames = DoMethodA(o, (Msg)&m);
        ObtainSemaphore(&st->lock);
        if (m.oirs_Frames == 0)
            st->eof = TRUE;
        else
            st->head += m.oirs_Frames * st->frameBytes;
        ReleaseSemaphore(&st->lock);
        Signal(st->reader, 1UL << st->readerSig);
    }
    if (o)
        DisposeDTObject(o);
    Forbid();                                   /* the reader frees st once done is set */
    st->done = TRUE;
    Signal(st->reader, 1UL << st->readerSig);
}

static void stopStream(OOGFOSDTSound *s)
{
    struct Stream *st = s->stream;
    if (!st)
        return;
    if (st->worker) {
        st->quit = TRUE;
        Signal(st->worker, SIGBREAKF_CTRL_C);
        while (!st->done)
            Wait(1UL << st->readerSig);
    }
    FreeSignal(st->readerSig);
    FreeVec(st->ring);
    FreeVec(st);
    s->stream = NULL;
}

static int startStream(OOGFOSDTSound *s, unsigned long ringBytes)
{
    struct Stream *st = AllocVec(sizeof *st, MEMF_PUBLIC | MEMF_CLEAR);
    struct Process *p;

    if (!st)
        return 0;
    InitSemaphore(&st->lock);
    st->frameBytes = 2UL * (ULONG)s->channels;
    st->size = ringBytes - ringBytes % st->frameBytes;
    st->ring = AllocVec(st->size, MEMF_PUBLIC);
    st->reader = FindTask(NULL);
    st->readerSig = AllocSignal(-1);
    st->path = s->path;
    s->stream = st;
    if (!st->ring || st->readerSig == -1) {
        if (st->readerSig != -1)
            FreeSignal(st->readerSig);
        FreeVec(st->ring);
        FreeVec(st);
        s->stream = NULL;
        return 0;
    }
    Forbid();                                   /* tc_UserData set before it runs */
    p = CreateNewProcTags(NP_Entry, (ULONG)streamWorker, NP_Name, (ULONG)"Oolite sound stream",
                          NP_StackSize, 65536, NP_Priority, 0, NP_Output, 0, NP_Input, 0,
                          NP_CloseOutput, FALSE, NP_CloseInput, FALSE, TAG_DONE);
    if (p) {
        p->pr_Task.tc_UserData = st;
        st->worker = &p->pr_Task;
    }
    Permit();
    if (!p) {
        stopStream(s);
        return 0;
    }
    return 1;
}

unsigned long OOGFOSDTSoundStream(OOGFOSDTSound *s, short *buffer, unsigned long maxFrames, unsigned long ringBytes)
{
    struct Stream *st;
    ULONG want, got = 0;

    if (!buffer || !maxFrames)
        return 0;
    if (!s->stream && !startStream(s, ringBytes))
        return OOGFOSDTSoundRead(s, buffer, maxFrames);     /* no process: decode here */
    if (s->o) {                                 /* the stream's process has its own object */
        DisposeDTObject(s->o);
        s->o = NULL;
    }
    st = s->stream;
    want = maxFrames * st->frameBytes;
    while (got < want) {
        ULONG avail, at, n;
        BOOL end;
        ObtainSemaphore(&st->lock);
        avail = st->head - st->tail;
        at = st->tail % st->size;
        end = st->eof || st->failed || st->done;
        n = want - got;
        if (n > avail)
            n = avail;
        if (n > st->size - at)
            n = st->size - at;
        if (n) {
            CopyMem(st->ring + at, (UBYTE *)buffer + got, n);
            st->tail += n;
            got += n;
        }
        ReleaseSemaphore(&st->lock);
        if (n) {
            Signal(st->worker, SIGBREAKF_CTRL_F);   /* room again */
            continue;
        }
        if (end)
            break;
        Wait(1UL << st->readerSig);             /* only when it is behind */
    }
    return got / st->frameBytes;
}

int OOGFOSDTSoundRewind(OOGFOSDTSound *s)
{
    stopStream(s);
    if (s->o)
        DisposeDTObject(s->o);
    s->o = NULL;
    return openObject(s);
}

void OOGFOSDTSoundClose(OOGFOSDTSound *s)
{
    if (!s)
        return;
    stopStream(s);
    if (s->o)
        DisposeDTObject(s->o);
    free(s->path);
    free(s);
}
