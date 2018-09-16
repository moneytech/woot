#include <cpu.h>
#include <malloc.h>
#include <new.h>
#include <sysdefs.h>
#include <stdlib.h>
#include <types.h>

void *operator new(unsigned int size)
{
    return malloc(size);
}

void *operator new[](unsigned int size)
{
    return malloc(size);
}

void *operator new(size_t size, size_t alignment)
{
    return memalign(alignment, size);
}

void *operator new[](size_t size, size_t alignment)
{
    return memalign(alignment, size);
}

void operator delete(void *ptr, unsigned int size)
{
    free(ptr);
}

void operator delete(void *ptr)
{
    free(ptr);
}

void operator delete[](void *ptr, unsigned int size)
{
    free(ptr);
}

void operator delete[](void *ptr)
{
    free(ptr);
}

void *operator new(size_t size, CustomAllocator allocator)
{
    return allocator(size);
}

void *operator new[](size_t size, CustomAllocator allocator)
{
    return allocator(size);
}

void operator delete(void *ptr, CustomDeallocator deallocator)
{
    deallocator(ptr);
}

void operator delete[](void *ptr, CustomDeallocator deallocator)
{
    deallocator(ptr);
}

extern "C" void __cxa_pure_virtual()
{
    cpuSystemHalt(0xCACACACA);
}
