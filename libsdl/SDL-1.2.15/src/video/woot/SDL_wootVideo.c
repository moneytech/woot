#include "SDL_video.h"
#include "../SDL_sysvideo.h"

static SDL_VideoDevice *WOOT_CreateDevice(int devindex)
{
}

static void WOOT_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}


