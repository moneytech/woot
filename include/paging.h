#ifndef PAGING_H
#define PAGING_H

#include <multiboot.h>
#include <types.h>

class Paging
{
public:
    static void Initialize(size_t ramSize);
    static void BuildAddressSpace(uintptr_t pd); // builds new address space
    static uintptr_t GetAddressSpace(); // returns current address space
    static void FlushTLB();
    static void InvalidatePage(uintptr_t addr);
    static bool MapPage(uintptr_t pd, uintptr_t va, uintptr_t pa, bool ps4m, bool user, bool write);
    static bool UnMapPage(uintptr_t pd, uintptr_t va, bool ps4m);
    static bool MapPages(uintptr_t pd, uintptr_t va, uintptr_t pa, bool ps4m, bool user, bool write, size_t n);
    static bool UnMapPages(uintptr_t pd, uintptr_t va, bool ps4m, size_t n);
    static uintptr_t GetPhysicalAddress(uintptr_t pd, uintptr_t va);
    static void UnmapRange(uintptr_t pd, uintptr_t startVA, size_t rangeSize);
    static void CloneRange(uintptr_t dstPd, uintptr_t srcPd, uintptr_t startVA, size_t rangeSize);

    static uintptr_t AllocPage();
    static uintptr_t AllocPages(size_t n);
    static bool FreePage(uintptr_t pa);
    static bool FreePages(uintptr_t pa, size_t n);
};

extern "C" void initializePaging(multiboot_info_t *mbootInfo);

int open(const char *filename, int flags);
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);

#endif // PAGING_H
