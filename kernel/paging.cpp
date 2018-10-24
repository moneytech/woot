#include <bitmap.h>
#include <cpu.h>
#include <gdt.h>
#include <malloc.h>
#include <paging.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdefs.h>
#include <thread.h>

// 4k region is at the end of kernel address and
// allows for accessing whole physical memory using 4k windows
// from any address space that maps kernel address space

static dword *kernelPageDir = nullptr;
static Bitmap *pageBitmap = nullptr;
static dword *kernel4kPT = nullptr;
uintptr_t kernelAddressSpace = 0;
static uintptr_t kernel4kVA = KERNEL_BASE + KERNEL_SPACE_SIZE - (4 << 20);

static void *map4k(uint slot, uintptr_t pa)
{
    kernel4kPT[slot] = pa | 0x03;
    uintptr_t va = kernel4kVA + slot * PAGE_SIZE;
    Paging::InvalidatePage(va);
    return (void *)va;
}

static void *alloc4k(uintptr_t pa)
{
    for(uint slot = 0; slot < 1024; ++slot)
    {
        if(!(kernel4kPT[slot] & 1))
            return map4k(slot, pa);
    }
    printf("paging: alloc4k() FAILED!\n");
    return nullptr;
}

static void free4k(void *ptr)
{
    uintptr_t p = (uintptr_t)ptr;
    p &= ~(PAGE_SIZE - 1);
    if(p < kernel4kVA || p >= (kernel4kVA + (2 << 20)))
        return;
    uint slot = (p - kernel4kVA) / PAGE_SIZE;
    kernel4kPT[slot] = 0;
}

void Paging::Initialize(size_t ramSize)
{
    bool ints = cpuDisableInterrupts();
    kernelPageDir = (dword *)valloc(PAGE_SIZE);
    memset(kernelPageDir, 0, PAGE_SIZE);
    pageBitmap = new Bitmap(ramSize / PAGE_SIZE, false);
    kernel4kPT = (dword *)valloc(PAGE_SIZE);
    memset(kernel4kPT, 0, PAGE_SIZE);

    kernelAddressSpace = ((uintptr_t)kernelPageDir) - KERNEL_BASE;
    GDT::MainTSS.CR3 = kernelAddressSpace;

    // identity map first 3 gigs
    for(uintptr_t va = 0, pdidx = 0; va < KERNEL_BASE; va += LARGE_PAGE_SIZE, ++pdidx)
        kernelPageDir[pdidx] = va | 0x83;
    // map mmio space
    for(uintptr_t va = MMIO_BASE; va; va += LARGE_PAGE_SIZE)
        kernelPageDir[va >> 22] = va | 0x83;
    // map kernel space
    for(uintptr_t va = 0; va < KERNEL_SPACE_SIZE; va += LARGE_PAGE_SIZE)
        kernelPageDir[(KERNEL_BASE + va) >> 22] = va | 0x83;
    // map 4k region
    kernelPageDir[kernel4kVA >> 22] = (((uintptr_t)kernel4kPT) - KERNEL_BASE) | 0x03;
    // unmap space for modules
    for(uintptr_t va = 0; va < MODULES_SPACE_SIZE; va += LARGE_PAGE_SIZE)
        kernelPageDir[(MODULES_BASE + va) >> 22] = 0;

    cpuSetCR3(kernelAddressSpace);

    // map vm86 memory and unmap address 0 for nullptr
    // dereferences to cause page faults
    for(uintptr_t va = 0; va < (1 << 20); va += PAGE_SIZE)
        MapPage(kernelAddressSpace, va, va, false, true, true);
    UnMapPage(kernelAddressSpace, 0, false);

    cpuRestoreInterrupts(ints);
}

void Paging::BuildAddressSpace(uintptr_t pd)
{
    dword *PD = (dword *)alloc4k(pd);
    dword *cPD = (dword *)alloc4k(GetAddressSpace());

    // zero user address space and clone current kernel address space
    uint i;
    for(i = 0; i < (KERNEL_BASE / LARGE_PAGE_SIZE); ++i)
        PD[i] = 0;
    for(; i < 1024; ++i)
        PD[i] = cPD[i];

    free4k(cPD);
    free4k(PD);
}

uintptr_t Paging::GetAddressSpace()
{
    return cpuGetCR3();
}

void Paging::FlushTLB()
{
    cpuSetCR3(cpuGetCR3());
}

void Paging::InvalidatePage(uintptr_t addr)
{
    cpuInvalidatePage(addr);
}

bool Paging::MapPage(uintptr_t pd, uintptr_t va, uintptr_t pa, bool ps4m, bool user, bool write)
{
    bool cs = cpuDisableInterrupts();
    va &= ~(PAGE_SIZE - 1);
    pa &= ~(PAGE_SIZE - 1);

    dword *pdir = (dword *)alloc4k(pd);
    if(!pdir)
    {
        cpuRestoreInterrupts(cs);
        return false;
    }
    uint pdidx = va >> 22;

    uintptr_t rwflag = write ? 0x02 : 0;
    uintptr_t uflag = user ? 0x04 : 0;
    //uintptr_t uflag = 0x04; // for testing

    if(ps4m)
    {
        va &= ~(LARGE_PAGE_SIZE - 1);
        pa &= ~(LARGE_PAGE_SIZE - 1);
        pdir[pdidx] = pa | 0x81 | rwflag | uflag;
        InvalidatePage(va);
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return true;
    }

    dword pdflags = pdir[pdidx] & 0x3F;

    if(pdir[pdidx] & 0x80)
    { // divide 4m mapping to 1024 4k mappings
        uintptr_t pdpa = pdir[pdidx] & ~(LARGE_PAGE_SIZE - 1);
        uintptr_t newptpa = AllocPage();
        if(newptpa == ~0)
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        dword *newpt = (dword *)alloc4k(newptpa);
        for(uint i = 0; i < 1024; ++i)
            newpt[i] = (pdpa + i * PAGE_SIZE) | pdflags;
        pdir[pdidx] = newptpa | 0x07;
        free4k(newpt);
    }

    uintptr_t ptpa = pdir[pdidx] & ~(PAGE_SIZE - 1);
    dword *pt = nullptr;
    if(!ptpa && !(pdflags & 1))
    {
        ptpa = AllocPage();
        if(ptpa == ~0)
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        pt = (dword *)alloc4k(ptpa);
        memset(pt, 0, PAGE_SIZE);
        pdir[pdidx] = ptpa | 0x07;
    }
    free4k(pdir);

    pt = pt ? pt : (dword *)alloc4k(ptpa);
    uint ptidx = va >> 12 & 1023;
    pt[ptidx] = pa | 0x01 | rwflag | uflag;
    free4k(pt);

    cpuRestoreInterrupts(cs);
    return true;
}

bool Paging::UnMapPage(uintptr_t pd, uintptr_t va, bool ps4m)
{
    bool cs = cpuDisableInterrupts();
    va &= ~(PAGE_SIZE - 1);

    dword *pdir = (dword *)alloc4k(pd);
    if(!pdir)
    {
        cpuRestoreInterrupts(cs);
        return false;
    }
    uint pdidx = va >> 22;

    if(!(pdir[pdidx] & 1))
    { // not present in page directory
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return false;
    }

    if(ps4m)
    { // unmap 4m mapping
        va &= ~(LARGE_PAGE_SIZE - 1);
        if(!(pdir[pdidx] & 0x80))
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        pdir[pdidx] = 0;
        InvalidatePage(va);
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return true;
    }

    dword pdflags = pdir[pdidx] & 0x3F;

    if(pdir[pdidx] & 0x80)
    { // divide 4m mapping to 1024 4k mappings
        uintptr_t pdpa = pdir[pdidx] & ~(LARGE_PAGE_SIZE - 1);
        uintptr_t newptpa = AllocPage();
        if(newptpa == ~0)
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        dword *newpt = (dword *)alloc4k(newptpa);
        for(uint i = 0; i < 1024; ++i)
            newpt[i] = (pdpa + i * PAGE_SIZE) | pdflags;
        pdir[pdidx] = newptpa | 0x07;
        free4k(newpt);
    }

    uintptr_t ptpa = pdir[pdidx] & ~(PAGE_SIZE - 1);
    dword *pt = (dword *)alloc4k(ptpa);
    if(!pt)
    {
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return false;
    }
    uint ptidx = va >> 12 & 1023;
    dword ptflags = pt[ptidx] & 0x3F;
    if(!(ptflags & 1))
    { // not present in page table
        free4k(pt);
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return false;
    }

    pt[ptidx] = 0;
    bool freept = true;
    for(uint i = 0; i < 1024; ++i)
    {
        if(pt[i] & 1)
        {
            freept = false;
            break;
        }
    }

    free4k(pt);

    if(freept)
    {
        FreePage(ptpa);
        pdir[pdidx] = 0;
    }

    free4k(pdir);
    cpuRestoreInterrupts(cs);
    return true;
}

bool Paging::MapPages(uintptr_t pd, uintptr_t va, uintptr_t pa, bool ps4m, bool user, bool write, size_t n)
{
    for(uintptr_t i = 0; i < n; ++i)
    {
        if(!MapPage(pd, va, pa, ps4m, user, write))
            return false;
        size_t incr = ps4m ? LARGE_PAGE_SIZE : PAGE_SIZE;
        va += incr;
        pa += incr;
    }
    return true;
}

bool Paging::UnMapPages(uintptr_t pd, uintptr_t va, bool ps4m, size_t n)
{
    for(uintptr_t i = 0; i < n; ++i)
    {
        if(!UnMapPage(pd, va, ps4m))
            return false;
        va += ps4m ? LARGE_PAGE_SIZE : PAGE_SIZE;
    }
    return true;
}

uintptr_t Paging::GetPhysicalAddress(uintptr_t pd, uintptr_t va)
{
    bool cs = cpuDisableInterrupts();
    dword *pdir = (dword *)alloc4k(pd);
    if(!pdir)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uint pdidx = va >> 22;
    dword pdflags = pdir[pdidx] & 0x3F;

    if(!(pdflags & 1))
    { // not present in page directory
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return ~0;
    }

    if(pdir[pdidx] & 0x80)
    { // 4m mapping
        uintptr_t offs = va & (LARGE_PAGE_SIZE - 1);
        uintptr_t pa = (pdir[pdidx] & ~(LARGE_PAGE_SIZE - 1)) + offs;
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return pa;
    }

    uintptr_t ptpa = pdir[pdidx] & ~(PAGE_SIZE - 1);
    free4k(pdir);
    dword *pt = (dword *)alloc4k(ptpa);
    if(!pt)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uint ptidx = va >> 12 & 1023;
    if(!(pt[ptidx] & 1))
    { // not present in page table
        free4k(pt);
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uintptr_t pa = pt[ptidx] & ~(PAGE_SIZE - 1);
    free4k(pt);
    cpuRestoreInterrupts(cs);
    return pa + (va & (PAGE_SIZE - 1));
}

void Paging::UnmapRange(uintptr_t pd, uintptr_t startVA, size_t rangeSize)
{
    bool cs = cpuDisableInterrupts();
    uint startPD = startVA >> 22;
    uint endPD = (startVA + rangeSize + LARGE_PAGE_SIZE - 1) >> 22;
    dword *PD = (dword *)alloc4k(pd);
    if(!PD)
    {
        cpuRestoreInterrupts(cs);
        return;
    }

    for(uint pdidx = startPD; pdidx < endPD; ++pdidx)
    {
        for(uint ptidx = 0; ptidx < PAGE_SIZE / 4; ++ptidx)
        {
            uintptr_t va = pdidx << 22 | ptidx << 12;
            if(va < startVA || (va >= (startVA + rangeSize)))
                continue;
            dword *PDE = PD + pdidx;
            if(!(*PDE & 1)) continue;
            if(*PDE & 0x80)
            { // unmap large page
                *PDE = 0;
                continue;
            }
            uintptr_t ptpa = *PDE & ~(PAGE_SIZE - 1);
            dword *PT = (dword *)alloc4k(ptpa);
            if(!PT)
            {
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            dword *PTE = PT + ptidx;
            uintptr_t pa = *PTE & ~(PAGE_SIZE - 1);
            FreePage(pa);
            *PTE = 0;
            bool freept = true;
            for(uint i = 0; i < 1024; ++i)
            {
                if(PT[i] & 1)
                {
                    freept = false;
                    break;
                }
            }
            free4k(PT);

            if(freept)
            {
                FreePage(ptpa);
                *PD = 0;
            }
        }
    }
    free4k(PD);
    cpuRestoreInterrupts(cs);
}

void Paging::CloneRange(uintptr_t dstPd, uintptr_t srcPd, uintptr_t startVA, size_t rangeSize)
{
    bool cs = cpuDisableInterrupts();
    uint startPD = startVA >> 22;
    uint endPD = (startVA + rangeSize + LARGE_PAGE_SIZE - 1) >> 22;
    dword *PD = (dword *)alloc4k(cpuGetCR3());
    if(!PD)
    {
        cpuRestoreInterrupts(cs);
        return;
    }

    for(uint pdidx = startPD; pdidx < endPD; ++pdidx)
    {
        for(uint ptidx = 0; ptidx < PAGE_SIZE / 4; ++ptidx)
        {
            uintptr_t va = pdidx << 22 | ptidx << 12;
            if(va < startVA || (va >= (startVA + rangeSize)))
                continue;
            dword *PDE = PD + pdidx;
            if(!(*PDE & 1)) continue;
            if(*PDE & 0x80)
            { // clone large page
                printf("pagingCloneRange: large pages not supported yet\n");
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            uintptr_t ptpa = *PDE & ~(PAGE_SIZE - 1);
            dword *PT = (dword *)alloc4k(ptpa);
            if(!PT)
            {
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            dword *PTE = PT + ptidx;
            dword ptflags = *PTE & 0x3F;
            uintptr_t pa = *PTE & ~(PAGE_SIZE - 1);
            uintptr_t dpa = AllocPage();
            if(dpa == ~0)
            {
                free4k(PT);
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            free4k(PT);
            // TODO: implement copy on write instead
            byte *src = (byte *)alloc4k(pa);
            if(!src)
            {
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            byte *dst = (byte *)alloc4k(dpa);
            if(!dst)
            {
                free4k(src);
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            memcpy(dst, src, PAGE_SIZE);
            free4k(dst);
            free4k(src);
            MapPage(dstPd, va, dpa, false, ptflags & 4 ? true : false, ptflags & 2 ? true : false);
        }
    }
    free4k(PD);
    cpuRestoreInterrupts(cs);
}

uintptr_t Paging::AllocPage()
{
    bool cs = cpuDisableInterrupts();
    uint bit = pageBitmap->FindLast(false);
    if(bit == ~0)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uintptr_t addr = bit * PAGE_SIZE;
    if(addr <= GetPhysicalAddress(cpuGetCR3(), (uintptr_t)sbrk(0)))
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    pageBitmap->SetBit(bit, true);
    cpuRestoreInterrupts(cs);
    return addr;
}

uintptr_t Paging::AllocPages(size_t n)
{
    bool cs = cpuDisableInterrupts();
    uint bit = pageBitmap->FindLast(false, n);
    if(bit == ~0)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    for(uint i = 0; i < n; ++i)
        pageBitmap->SetBit(bit + i, true);
    uintptr_t addr = bit * PAGE_SIZE;
    if(addr <= GetPhysicalAddress(cpuGetCR3(), (uintptr_t)sbrk(0)))
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    cpuRestoreInterrupts(cs);
    return addr;
}

bool Paging::FreePage(uintptr_t pa)
{
    uint bit = pa / PAGE_SIZE;
    bool cs = cpuDisableInterrupts();
    bool state = pageBitmap->GetBit(bit);
    if(!state)
    {
        cpuRestoreInterrupts(cs);
        return false;
    }
    pageBitmap->SetBit(bit, false);
    cpuRestoreInterrupts(cs);
    return true;
}

bool Paging::FreePages(uintptr_t pa, size_t n)
{
    for(uint i = 0; i < n; ++i)
    {
        if(!FreePage(pa))
            return false;
        pa += PAGE_SIZE;
    }
    return true;
}
