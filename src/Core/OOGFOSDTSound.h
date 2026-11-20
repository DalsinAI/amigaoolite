/* Sounds through the datatypes on GFOS: the file is opened as a datatype object
 * with OIA_Reader and its PCM read a piece at a time with OIM_READSOUND
 * (gfopendatatypes, datatypes/gfopenimage.h). gfopensound.datatype hands
 * Ogg Vorbis to the services card (AkikoPlus) and decodes on the 68k only
 * without one. Plain C, so the Objective-C side never sees the Amiga headers.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OOGFOS_DTSOUND_H
#define OOGFOS_DTSOUND_H

typedef struct OOGFOSDTSound OOGFOSDTSound;

/* NULL when the datatypes can't open it as a readable sound. */
OOGFOSDTSound *OOGFOSDTSoundOpen(const char *path);
/* 1 or 2 */
int OOGFOSDTSoundChannels(const OOGFOSDTSound *s);
long OOGFOSDTSoundRate(const OOGFOSDTSound *s);
/* frames in the whole sound, as the datatype says (0 when it doesn't know) */
unsigned long OOGFOSDTSoundFrames(const OOGFOSDTSound *s);
/* up to maxFrames frames, 16-bit signed big-endian (the 68k's order),
 * interleaved when stereo; 0 at the end or on an error */
unsigned long OOGFOSDTSoundRead(OOGFOSDTSound *s, short *buffer, unsigned long maxFrames);
/* back to the start (the object is made again); 0 when that fails */
int OOGFOSDTSoundRewind(OOGFOSDTSound *s);
void OOGFOSDTSoundClose(OOGFOSDTSound *s);

/* Streaming (music): a process of its own ("Oolite sound stream") decodes
 * ahead into a ring of ringBytes, so the game's task only copies. The first
 * call starts it; OOGFOSDTSoundRewind and OOGFOSDTSoundClose stop it. The
 * same 16-bit frames as OOGFOSDTSoundRead; 0 at the end. Call it, Rewind and
 * Close from one task. */
unsigned long OOGFOSDTSoundStream(OOGFOSDTSound *s, short *buffer, unsigned long maxFrames, unsigned long ringBytes);

#endif
