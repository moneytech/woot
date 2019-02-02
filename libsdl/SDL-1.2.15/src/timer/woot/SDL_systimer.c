#include "SDL_config.h"

#if defined(SDL_TIMER_WOOT)

#include "SDL_timer.h"
#include "../SDL_timer_c.h"

#include <woot/time.h>

static unsigned long long tickFreq = 0;
static unsigned long long startTicks = 0;

void SDL_StartTicks(void)
{
    startTicks = tmGetTicks();
}

Uint32 SDL_GetTicks(void)
{
    if(!tickFreq) tickFreq = tmGetTickFreq();
    return 1000 * (tmGetTicks() - startTicks) / tickFreq;
}

void SDL_Delay(Uint32 ms)
{
    tmSleep(ms);
}

static int RunTimer(void *unused)
{
    return -1;
}

int SDL_SYS_TimerInit(void)
{
    return -1;
}

void SDL_SYS_TimerQuit(void)
{
}

int SDL_SYS_StartTimer(void)
{
    return -1;
}

void SDL_SYS_StopTimer(void)
{
    return;
}

#endif /* SDL_TIMER_WOOT */
