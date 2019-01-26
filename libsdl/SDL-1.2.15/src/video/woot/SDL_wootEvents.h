#ifndef SDL_WOOTEVENTS_H
#define SDL_WOOTEVENTS_H

#include "SDL_keyboard.h"
#include "SDL_wootVideo.h"

extern SDLKey vkToKeySym[];

void WOOT_InitOSKeymap(_THIS);
void WOOT_PumpEvents(_THIS);

#endif // SDL_WOOTEVENTS_H
