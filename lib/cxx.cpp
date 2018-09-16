#include <cpu.h>
#include <stdlib.h>
#include <types.h>

void *operator new(size_t size)
{
    return malloc(size);
}

void *operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void *ptr, size_t size)
{
    free(ptr);
}

void operator delete[](void *ptr, size_t size)
{
    free(ptr);
}

extern "C" void __cxa_pure_virtual()
{
    cpuSystemHalt(0xCACACACA);
}
