#include <cpu.h>
#include <malloc.h>
#include <new.h>
#include <sysdefs.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

void *operator new(unsigned int size)
{
    return calloc(1, size);
}

void *operator new[](unsigned int size)
{
    return calloc(1, size);
}

void *operator new(size_t size, size_t alignment)
{
    void *ptr = memalign(alignment, size);
    memset(ptr, 0, size);
    return ptr;
}

void *operator new[](size_t size, size_t alignment)
{
    void *ptr = memalign(alignment, size);
    memset(ptr, 0, size);
    return ptr;
}

void *operator new(size_t size, void *ptr)
{
    memset(ptr, 0, size);
    return ptr;
}

void *operator new[](size_t size, void *ptr)
{
    memset(ptr, 0, size);
    return ptr;
}

void operator delete(void *ptr, unsigned int size)
{
    free(ptr);
}

void operator delete(void *ptr)
{
    free(ptr);
}

void operator delete(void *ptr, void *place)
{
    // do nothing
}

void operator delete[](void *ptr, unsigned int size)
{
    free(ptr);
}

void operator delete[](void *ptr)
{
    free(ptr);
}

void operator delete[](void *ptr, void *place)
{
    // do nothing
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
