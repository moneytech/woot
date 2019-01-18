#include <cpu.h>
#include <stdlib.h>

static uint32_t rsx = 0x1BADB002, rsy = 0xBAADF00D, rsz = 0xB16B00B2, rsw = 0x10121337;

void srand(uint seed)
{
    bool ints = cpuDisableInterrupts();
    rsx = 0x1BADB002 ^ seed;
    rsy = 0xBAADF00D ^ seed;
    rsz = 0xB16B00B2 ^ seed;
    rsw = 0x10121337 ^ seed;
    cpuRestoreInterrupts(ints);
}

uint rand()
{
    bool ints = cpuDisableInterrupts();
    uint32_t t = rsx ^ (rsx << 11);
    rsx = rsy; rsy = rsz; rsz = rsw;
    rsw = rsw ^ (rsw >> 19) ^ t ^ (t >> 8);
    cpuRestoreInterrupts(ints);
    return rsw;
}

void abort()
{
    cpuSystemHalt(0xAB07AB07);
    for(;;);
}

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
