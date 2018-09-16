#include <cpu.h>
#include <stdlib.h>
#include <time.h>

static uint32_t rsx = 0x1BADB002, rsy = 0xBAADF00D, rsz = 0xB16B00B2, rsw = 0x10121337;

void srand(uint seed)
{
    uint32_t t = time(0);
    rsx = 0x1BADB002 ^ t;
    rsy = 0xBAADF00D ^ t;
    rsz = 0xB16B00B2 ^ t;
    rsw = 0x10121337 ^ t;
}

uint rand()
{
    uint32_t t = rsx ^ (rsx << 11);
    rsx = rsy; rsy = rsz; rsz = rsw;
    rsw = rsw ^ (rsw >> 19) ^ t ^ (t >> 8);
    return rsw;
}

void abort()
{
    cpuSystemHalt(0xAB07AB07);
    for(;;);
}

// FIXME: Just a temporary solution
extern void *_bss_end;
void *sbrk(intptr_t incr)
{
    static uintptr_t curPtr = (uintptr_t)&_bss_end;
    bool ints = cpuDisableInterrupts();
    uintptr_t oldPtr = curPtr;
    curPtr += incr;
    cpuRestoreInterrupts(ints);
    return (void *)oldPtr;
}
