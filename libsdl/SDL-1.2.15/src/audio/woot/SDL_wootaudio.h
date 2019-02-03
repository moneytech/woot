#include "SDL_config.h"

#ifndef SDL_WOOTAUDIO_H
#define SDL_WOOTAUDIO_H

#include "SDL_audio.h"
#include "../SDL_sysaudio.h"

#define _THIS SDL_AudioDevice *this

struct SDL_PrivateAudioData
{
    int deviceId;
    Uint8 *buffer;
};

#endif // SDL_WOOTAUDIO_H
