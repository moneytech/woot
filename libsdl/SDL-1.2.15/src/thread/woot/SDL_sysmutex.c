#include "SDL_config.h"

#include "SDL_sysmutex_c.h"
#include "SDL_thread.h"

#include <woot/thread.h>

SDL_mutex *SDL_CreateMutex(void)
{
    SDL_mutex *mutex = (SDL_mutex *)SDL_calloc(1, sizeof(SDL_mutex));
    if(!mutex) return NULL;
    mutex->id = thrMutexCreate();
    if(mutex->id < 0)
    {
        SDL_DestroyMutex(mutex);
        return NULL;
    }
    return mutex;
}

int SDL_mutexP(SDL_mutex *mutex)
{
    if(!mutex || mutex->id < 0)
        return -1;
    return thrMutexAcquire(mutex->id, 0, 0) < 0 ? -1 : 0;
}

int SDL_mutexV(SDL_mutex *mutex)
{
    if(!mutex || mutex->id < 0)
        return -1;
    return thrMutexRelease(mutex->id) < 0 ? -1 : 0;
}

void SDL_DestroyMutex(SDL_mutex *mutex)
{
    if(!mutex) return;
    if(mutex->id >= 0)
        thrMutexDelete(mutex->id);
    SDL_free(mutex);
}
