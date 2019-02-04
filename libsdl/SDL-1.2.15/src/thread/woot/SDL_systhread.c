#include "SDL_config.h"

#include "SDL_thread.h"
#include "SDL_systhread_c.h"
#include "../SDL_systhread.h"
#include "../SDL_thread_c.h"

#include <woot/thread.h>

static int threadEntry(void *data)
{
    SDL_RunThread(data);
    return 0;
}

int SDL_SYS_CreateThread(SDL_Thread *thread, void *args)
{
    if(!thread) return -1;
    thread->handle.finished = thrSemaphoreCreate(0);
    thread->handle.id = thrCreate(threadEntry, thread->handle.finished, &thread->status, args);
    thrResume(thread->handle.id);
}

void SDL_SYS_SetupThread(void)
{
    // nothing to be done here
}

Uint32 SDL_ThreadID(void)
{
    return thrGetCurrentID();
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{
    if(!thread) return;
    thrSemaphoreWait(thread->handle.finished, 0, 0);
}

void SDL_SYS_KillThread(SDL_Thread *thread)
{
    if(!thread) return;
    thrDelete(thread->handle.id, -1);
    thrSemaphoreDelete(thread->handle.finished);
}
