#ifndef NEW_H
#define NEW_H

#include <types.h>

extern "C++"
{

void *operator new(unsigned int size);
void *operator new[](unsigned int size);
void *operator new(size_t size, size_t alignment); // aligned version of new
void *operator new[](size_t size, size_t alignment); // aligned version of new[]
void *operator new(size_t size, void *ptr); // placement new
void *operator new[](size_t size, void *ptr); // placement new[]
void operator delete(void *ptr, size_t size);
void operator delete(void *ptr);
void operator delete(void *ptr, void *place); // 'placement delete'
void operator delete[](void *ptr, size_t size);
void operator delete[](void *ptr);
void operator delete[](void *ptr, void *place); // 'placement delete[]'

typedef void *(*CustomAllocator)(size_t size);
typedef void (*CustomDeallocator)(void *ptr);

void *operator new(size_t size, CustomAllocator allocator);
void *operator new[](size_t size, CustomAllocator allocator);
void operator delete(void *ptr, CustomDeallocator deallocator);
void operator delete[](void *ptr, CustomDeallocator deallocator);

}

#endif // NEW_H
