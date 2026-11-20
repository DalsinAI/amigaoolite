/* OpenAL tone probe: altone [DEVICE] [SECONDS]
 * Opens DEVICE (default: OpenAL's own choice; "AHI Voices" or "AHI Mix"),
 * says which device it got, and plays a 440 Hz tone, as Oolite's mixer
 * plays a sound. For hearing whether OpenAL reaches AHI on a card.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <AL/al.h>
#include <AL/alc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *want = argc > 1 && argv[1][0] ? argv[1] : NULL;
    int secs = argc > 2 ? atoi(argv[2]) : 3;
    printf("default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));
    ALCdevice *dev = alcOpenDevice(want);
    if (!dev) { printf("alcOpenDevice(%s) failed\n", want ? want : "NULL"); return 20; }
    printf("opened: %s\n", alcGetString(dev, ALC_DEVICE_SPECIFIER));
    ALCcontext *ctx = alcCreateContext(dev, NULL);
    if (!ctx || !alcMakeContextCurrent(ctx)) { printf("no context\n"); return 20; }
    enum { RATE = 22050 };
    static short pcm[RATE];
    for (int i = 0; i < RATE; i++) pcm[i] = (short)(12000.0 * sin(2.0 * 3.14159265 * 440.0 * i / RATE));
    ALuint buf, src;
    alGenBuffers(1, &buf);
    alBufferData(buf, AL_FORMAT_MONO16, pcm, sizeof pcm, RATE);
    alGenSources(1, &src);
    alSourcei(src, AL_BUFFER, (ALint)buf);
    alSourcei(src, AL_LOOPING, AL_TRUE);
    alSourcePlay(src);
    printf("AL error after play: 0x%x\n", alGetError());
    for (int t = 0; t < secs * 10; t++) {
        alcProcessContext(ctx);
        usleep(100000);
    }
    ALint state = 0, off = 0;
    alGetSourcei(src, AL_SOURCE_STATE, &state);
    alGetSourcei(src, AL_SAMPLE_OFFSET, &off);
    printf("state 0x%x (playing is 0x%x), offset %d\n", state, AL_PLAYING, off);
    alSourceStop(src);
    alDeleteSources(1, &src);
    alDeleteBuffers(1, &buf);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(dev);
    return 0;
}
