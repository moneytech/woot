#include <cpu.h>
#include <malloc.h>
#include <new.h>
#include <sysdefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

extern "C" uintptr_t __dso_handle;
uintptr_t __dso_handle = 0;

typedef void (*atexitHandler)(void *ptr, void *dso);
class atexitTableEntry
{
public:
    atexitHandler handler;
    void *ptr, *dso;
    bool handled;
    atexitTableEntry() : handler(nullptr), ptr(nullptr), dso(nullptr), handled(false) {}
    atexitTableEntry(atexitHandler handler, void *ptr, void *dso) :
        handler(handler), ptr(ptr), dso(dso), handled(false)
    {
    }
};
#define MAX_ATEXIT_HANDLERS 4096
static atexitTableEntry atexitTable[MAX_ATEXIT_HANDLERS];
static int atexitHandlers = 0;

extern "C" void __cxa_atexit(atexitHandler handler, void *ptr, void *dso)
{
    if(atexitHandlers >= MAX_ATEXIT_HANDLERS)
    {
        printf("[cxx] atexitHandlers >= MAX_ATEXIT_HANDLERS\n");
        return;
    }
    atexitTable[atexitHandlers++] = atexitTableEntry(handler, ptr, dso);
}

extern "C" void __cxa_finalize(atexitHandler handler)
{
    for(int i = atexitHandlers - 1; i > 0; --i)
    {
        atexitTableEntry *entry = atexitTable + i;
        if(entry->handled || !entry->handler)
            continue;
        if(!handler || (handler && entry->handler))
        {
            entry->handler(entry->ptr, entry->dso);
            entry->handled = true;
        }
    }
}

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
    memset(ptr, 0, size);
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
    memset(ptr, 0, size);
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
