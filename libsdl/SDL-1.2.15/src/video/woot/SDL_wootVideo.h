#ifndef SDL_WOOTVIDEO_H
#define SDL_WOOTVIDEO_H

#define _THIS SDL_VideoDevice *this

struct SDL_PrivateVideoData
{
    struct wmWindow *window;
    struct pmPixMap *pm;
};

#endif // SDL_WOOTVIDEO_H
