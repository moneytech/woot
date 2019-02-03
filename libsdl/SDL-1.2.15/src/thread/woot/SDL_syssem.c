#include "SDL_config.h"
#include "SDL_syssem_c.h"
#include "SDL_thread.h"

#include <woot/thread.h>

SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
    SDL_sem *sem = (SDL_sem *)SDL_calloc(1, sizeof(SDL_sem));
    if(!sem) return NULL;
    sem->id = thrMutexCreate();
    if(sem->id < 0)
    {
        SDL_DestroySemaphore(sem);
        return NULL;
    }
    return sem;
}

int SDL_SemTryWait(SDL_sem *sem)
{
    if(!sem || sem->id < 0)
        return -1;
    return thrSemaphoreWait(sem->id, 0, 1) < 0 ? SDL_MUTEX_TIMEDOUT : 0;
}

int SDL_SemWait(SDL_sem *sem)
{
    if(!sem || sem->id < 0)
        return -1;
    return thrSemaphoreWait(sem->id, 0, 0) < 0 ? -1 : 0;
}

int SDL_SemWaitTimeout(SDL_sem *sem, Uint32 timeout)
{
    if(!sem || sem->id < 0)
        return -1;
    return thrSemaphoreWait(sem->id, timeout, 0) < 0 ? -1 : 0;
}

Uint32 SDL_SemValue(SDL_sem *sem)
{
    if(!sem || sem->id < 0)
        return 0;
    int res = thrSemaphoreGetCount(sem->id);
    return res < 0 ? 0 : res;
}

int SDL_SemPost(SDL_sem *sem)
{
    if(!sem || sem->id < 0)
        return -1;
    return thrSemaphoreSignal(sem->id) < 0 ? -1 : 0;
}

void SDL_DestroySemaphore(SDL_sem *sem)
{
    if(!sem) return;
    if(sem->id >= 0)
        thrSemaphoreDelete(sem->id);
    SDL_free(sem);
}
