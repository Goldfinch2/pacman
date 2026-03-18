#include "sound.h"
#include "defines.h"

#define NUM_TRACKS 64

sound::Track* sound::mTracks;

sound::sound()
{
    mTracks = new Track[NUM_TRACKS];

    for (int i = 0; i < NUM_TRACKS; i++)
    {
        mTracks[i].data    = NULL;
        mTracks[i].playing = false;
        mTracks[i].speed   = 1.0;
        mTracks[i].vol     = 1.0f;
        mTracks[i].pos     = 0.0;
        mTracks[i].len     = 0;
        mTracks[i].loop    = false;
    }
}

sound::~sound()
{
    stopSound();

    for (int i = 0; i < NUM_TRACKS; i++)
    {
        if (mTracks[i].data)
            free(mTracks[i].data);
    }

    delete[] mTracks;

    SDL_CloseAudio();
}

void sound::bufferCallback(void* unused, Uint8* stream, int len)
{
    Sint16* buf = (Sint16*)stream;

    // Clear the output buffer
    memset(buf, 0, len);

    // Number of Sint16 samples in the buffer
    len /= 2;

    const int max_audioval =  ((1 << (16 - 1)) - 1);
    const int min_audioval = -(1 << (16 - 1));

    for (int i = 0; i < NUM_TRACKS; i++)
    {
        Track* track = &mTracks[i];

        if (!track->playing)
            continue;

        Sint16* data = (Sint16*)track->data;

        for (int s = 0; s < len; s += 2)
        {
            // Left channel (even sample)
            {
                float fPos  = (float)track->pos;
                int   iPos1 = (int)fPos;
                int   iPos2 = iPos1 + 1;

                if ((Uint32)iPos2 >= track->len)
                    iPos2 -= track->len;

                float interp = fPos - (float)iPos1;

                float v1 = data[iPos1] * (track->vol / 4.0f);
                float v2 = data[iPos2] * (track->vol / 4.0f);

                int mixed = (int)(v1 + interp * (v2 - v1)) + (int)buf[s];
                if (mixed > max_audioval) mixed = max_audioval;
                if (mixed < min_audioval) mixed = min_audioval;
                buf[s] = (Sint16)mixed;
            }

            // Right channel (odd sample)
            {
                float fPos  = (float)(track->pos + 1.0);
                int   iPos1 = (int)fPos;
                int   iPos2 = iPos1 + 1;

                if ((Uint32)iPos2 >= track->len)
                    iPos2 -= track->len;

                float interp = fPos - (float)iPos1;

                float v1 = data[iPos1] * (track->vol / 4.0f);
                float v2 = data[iPos2] * (track->vol / 4.0f);

                int mixed = (int)(v1 + interp * (v2 - v1)) + (int)buf[s + 1];
                if (mixed > max_audioval) mixed = max_audioval;
                if (mixed < min_audioval) mixed = min_audioval;
                buf[s + 1] = (Sint16)mixed;
            }

            track->pos += track->speed * 2.0;  // *2 for stereo interleaved (L,R pairs)

            if (track->pos >= (double)track->len)
            {
                if (track->loop)
                {
                    track->pos -= (double)track->len;
                }
                else
                {
                    track->pos     = 0;
                    track->playing = false;
                    break;
                }
            }
        }
    }
}

void sound::loadTrack(const char* file, int track, float volume, bool loop)
{
    SDL_AudioSpec wave;
    Uint8* data;
    Uint32 dlen;

    // Load the WAV file
    if (SDL_LoadWAV(file, &wave, &data, &dlen) == NULL)
    {
        fprintf(stderr, "Failed loading audio track %s: %s\n", file, SDL_GetError());
        return;
    }

    // Build a conversion to 16-bit stereo 44100 Hz
    SDL_AudioCVT cvt;
    SDL_BuildAudioCVT(&cvt,
        wave.format, wave.channels, wave.freq,
        AUDIO_S16, 2, 44100);

    cvt.buf = (Uint8*)malloc(dlen * cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_FreeWAV(data);

    if (SDL_ConvertAudio(&cvt) != 0)
    {
        fprintf(stderr, "Failed to convert track %d: %s\n", track, SDL_GetError());
    }

    // Free any previous data in this slot
    if (mTracks[track].data)
    {
        free(mTracks[track].data);
        mTracks[track].data = NULL;
    }

    SDL_LockAudio();
    mTracks[track].data    = cvt.buf;
    mTracks[track].len     = cvt.len_cvt / 2;  // length in Sint16 samples
    mTracks[track].pos     = 0;
    mTracks[track].loop    = loop;
    mTracks[track].vol     = volume;
    mTracks[track].playing = false;
    SDL_UnlockAudio();

}

void sound::startSound()
{
    SDL_AudioSpec desired;
    desired.freq     = 44100;
    desired.format   = AUDIO_S16;
    desired.channels = 2;
    desired.samples  = 1024;
    desired.callback = sound::bufferCallback;
    desired.userdata = NULL;

    if (SDL_OpenAudio(&desired, NULL) < 0)
    {
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
    }

    SDL_PauseAudio(0);
}

void sound::stopSound()
{
    SDL_PauseAudio(1);
}

void sound::playTrack(int track)
{
    SDL_LockAudio();
    mTracks[track].pos     = 0;
    mTracks[track].playing = true;
    SDL_UnlockAudio();
}

void sound::stopTrack(int track)
{
    SDL_LockAudio();
    mTracks[track].pos     = 0;
    mTracks[track].playing = false;
    SDL_UnlockAudio();
}

void sound::stopAllTracks()
{
    SDL_LockAudio();
    for (int i = 0; i < NUM_TRACKS; i++)
    {
        mTracks[i].pos     = 0;
        mTracks[i].playing = false;
    }
    SDL_UnlockAudio();
}

bool sound::isTrackPlaying(int track)
{
    bool playing;

    SDL_LockAudio();
    playing = mTracks[track].playing;
    SDL_UnlockAudio();

    return playing;
}

float sound::getTrackProgress(int track)
{
    float progress = 0.0f;

    SDL_LockAudio();
    if (mTracks[track].len > 0)
        progress = (float)(mTracks[track].pos / (double)mTracks[track].len);
    SDL_UnlockAudio();

    return progress;
}
