#ifndef SOUND_H
#define SOUND_H

#include "SDL.h"

class sound
{
public:
    sound();
    ~sound();

    void loadTrack(const char* file, int track, float volume, bool loop = false);
    void playTrack(int track);
    void stopTrack(int track);
    void stopAllTracks();
    void startSound();
    void stopSound();
    bool isTrackPlaying(int track);
    float getTrackProgress(int track);  // 0.0 to 1.0

private:
    static void bufferCallback(void* unused, Uint8* stream, int len);

    struct Track {
        Uint8* data;
        bool   loop;
        bool   playing;
        float  vol;
        double speed;
        double pos;
        Uint32 len;
    };

    static Track* mTracks;
};

#endif
