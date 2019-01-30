#include "SDL_config.h"

#if defined(SDL_TIMER_WOOT)

#include "SDL_timer.h"
#include "../SDL_timer_c.h"

static int ticks = 0;
static int startticks = 0;

void SDL_StartTicks(void)
{
    startticks = ticks;
}

Uint32 SDL_GetTicks (void)
{
    ticks += 7;
    return ticks - startticks;
}

void SDL_Delay (Uint32 ms)
{
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
