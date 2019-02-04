#include "SDL_config.h"
#include "SDL_wootaudio.h"

#include <woot/audio.h>

#define WOOTAUD_DRIVER_NAME         "woot"

static int WOOTAUD_Available(void);
static SDL_AudioDevice *WOOTAUD_CreateDevice(int devindex);
static void WOOTAUD_DeleteDevice(SDL_AudioDevice *device);

static int WOOTAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void WOOTAUD_WaitAudio(_THIS);
static void WOOTAUD_PlayAudio(_THIS);
static Uint8 *WOOTAUD_GetAudioBuf(_THIS);
static void WOOTAUD_CloseAudio(_THIS);

AudioBootStrap WOOTAUD_bootstrap =
{
    WOOTAUD_DRIVER_NAME, "WOOT audio driver",
    WOOTAUD_Available, WOOTAUD_CreateDevice
};

static int WOOTAUD_Available(void)
{
    return 1;
}

static SDL_AudioDevice *WOOTAUD_CreateDevice(int devindex)
{
    SDL_AudioDevice *this = SDL_calloc(1, sizeof(SDL_AudioDevice));
    if(!this)
    {
        SDL_OutOfMemory();
        return NULL;
    }
    this->hidden = (struct SDL_PrivateAudioData *)SDL_calloc(1, sizeof(struct SDL_PrivateAudioData));
    if(!this->hidden)
    {
        SDL_free(this);
        SDL_OutOfMemory();
        return NULL;
    }

    this->hidden->deviceId = devindex;

    this->OpenAudio = WOOTAUD_OpenAudio;
    this->WaitAudio = WOOTAUD_WaitAudio;
    this->PlayAudio = WOOTAUD_PlayAudio;
    this->GetAudioBuf = WOOTAUD_GetAudioBuf;
    this->CloseAudio = WOOTAUD_CloseAudio;
    this->free = WOOTAUD_DeleteDevice;

	return this;
}

static void WOOTAUD_DeleteDevice(SDL_AudioDevice *device)
{
    if(!device) return;
    if(device->hidden->buffer) SDL_free(device->hidden->buffer);
    if(device->hidden) SDL_free(device->hidden);
    SDL_free(device);
}

static void WOOTAUD_WaitAudio(_THIS)
{
    printf("%s: %s not implemented\n", __FILE__, __FUNCTION__);
}

static void WOOTAUD_PlayAudio(_THIS)
{
    auStartPlayback(this->hidden->deviceId);
    printf("%s: %s not implemented\n", __FILE__, __FUNCTION__);
}

static Uint8 *WOOTAUD_GetAudioBuf(_THIS)
{
    return this->hidden->buffer;
}

static void WOOTAUD_CloseAudio(_THIS)
{
    auCloseDevice(this->hidden->deviceId);
}

static int WOOTAUD_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
    int res = auOpenDevice(this->hidden->deviceId, spec->freq, spec->channels, spec->format & 0xFF, spec->samples) < 0 ? -1 : 0;
    this->hidden->buffer = (Uint8 *)SDL_calloc(1, auGetFrameSize(this->hidden->deviceId));
    return res;
}

