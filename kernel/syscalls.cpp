#include <cpu.h>
#include <debugstream.h>
#include <dentry.h>
#include <errno.h>
#include <file.h>
#include <inode.h>
#include <kwm.h>
#include <paging.h>
#include <process.h>
#include <stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <syscalls.h>
#include <sysdefs.h>
#include <thread.h>
#include <time.h>

#undef TIME_H
#include <../libc/include/time.h>

extern DebugStream debugStream;

Ints::Handler SysCalls::handler = { nullptr, SysCalls::isr, nullptr };

// syscall table
SysCalls::Callback SysCalls::callbacks[] =
{
    nullptr, sys_exit, nullptr, sys_read, sys_write, sys_open, sys_close, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_time, nullptr, nullptr, // 0 - 15
    nullptr, nullptr, nullptr, sys_lseek, sys_getpid, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 16 - 31
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_mkdir, nullptr, nullptr, nullptr, nullptr, nullptr, sys_brk, nullptr, nullptr, // 32 - 47
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 48 - 63
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 64 - 79
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 80 - 95
    nullptr, nullptr, nullptr, nullptr, sys_stat, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 96 - 111
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_fsync, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 112 - 127
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 128 - 143
    nullptr, nullptr, nullptr, nullptr, sys_fdatasync, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 144 - 159
    nullptr, nullptr, sys_nanosleep, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 160 - 175
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_getcwd, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 176 - 191
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 192 - 207
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 208 - 223
    sys_gettid, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 224 - 239
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 240 - 255
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 256 - 271
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 272 - 287
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 288 - 303
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 304 - 319
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 320 - 335
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 336 - 351
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 352 - 367
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 368 - 383
    nullptr, nullptr, sys_create_window, sys_show_window, sys_hide_window, sys_destroy_window, sys_draw_rectangle, sys_draw_filled_rectangle, sys_update_window, sys_redraw_window, sys_draw_line, sys_blit, sys_alpha_blit, sys_map_window, sys_invalidate_rect, sys_get_window_size, // 384 - 399
    sys_get_pixel_format, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 400 - 415
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 416 - 431
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 432 - 447
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 448 - 463
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 464 - 479
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 480 - 495
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr  // 496 - 511
};

bool SysCalls::isr(Ints::State *state, void *context)
{
    long args[] = { (long)state->EAX, (long)state->EBX, (long)state->ECX, (long)state->EDX,
                    (long)state->ESI, (long)state->EDI, (long)state->EBP };
    //printf("[syscalls] syscall(%d, %p, %p, %p, %p, %p)\n", args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
    if(args[0] >= MAX_SYSCALLS || !callbacks[args[0]])
    {
        printf("[syscalls] unknown syscall %u\n", args[0]);
        state->EAX = -EINVAL;
        return true;
    }
    state->EAX = callbacks[args[0]](args);
    return true;
}

long SysCalls::sys_exit(long *args) // 1
{
    Thread::Finalize(nullptr, args[1]);
    printf("[syscalls] sys_exit(): Thread::Finalize() returned\n");
    return 0; // should never happen
}

long SysCalls::sys_read(long *args) // 3
{
    if(!args[2])
        return -EINVAL;
    if(args[1] < 3)
        return debugStream.Read((void *)args[2], (int64_t)args[3]);
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    File *f = cp->GetFileDescriptor(args[1]);
    if(!f) return -EBADF;
    return f->Read((void *)args[2], args[3]);
}

long SysCalls::sys_write(long *args) // 4
{
    if(!args[2])
        return -EINVAL;
    if(args[1] < 3)
        return debugStream.Write((const void *)args[2], (int64_t)args[3]);
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    File *f = cp->GetFileDescriptor(args[1]);
    if(!f) return -EBADF;
    return f->Write((const void *)args[2], args[3]);
}

long SysCalls::sys_open(long *args) // 5
{
    //printf("sys_open('%s', %d)\n", (char *)args[1], args[2]);
    if(!args[1]) return -EINVAL;
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    return cp->Open((const char *)args[1], args[2]);
}

long SysCalls::sys_close(long *args) // 6
{
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    return cp->Close(args[1]);
}

long SysCalls::sys_time(long *args) // 13
{
    return time((time_t *)args[1]);
}

long SysCalls::sys_lseek(long *args) // 19
{
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    File *f = cp->GetFileDescriptor(args[1]);
    if(!f) return -EBADF;
    return f->Seek(args[2], args[3]);
}

long SysCalls::sys_getpid(long *args) // 20
{
    return Process::GetCurrent()->ID;
}

long SysCalls::sys_mkdir(long *args) // 39
{
    if(!args[1]) return -EINVAL;
    char *pathname = strdup((const char *)args[1]);
    char *basename = strrpbrk(pathname, PATH_SEPARATORS);
    char *dirname = basename ? pathname : (char *)".";
    if(basename) *basename++ = 0;
    else basename = pathname;
    File *dir = File::Open(dirname, O_DIRECTORY);
    if(!dir)
    {
        free(pathname);
        return -ENOENT;
    }
    if(!(S_ISDIR(dir->Mode)))
    {
        delete dir;
        free(pathname);
        return -ENOTDIR;
    }
    bool res = dir->Create(basename, S_IFDIR | (args[2] & 0777));
    delete dir;
    free(pathname);
    return res ? 0 : -EPERM;
}

long SysCalls::sys_brk(long *args) // 45
{
    //printf("sys_brk(%p)\n", args[1]);
    uintptr_t brk = args[1];
    Process *cp = Process::GetCurrent();
    if(!cp) return ~0;
    if(!cp->MemoryLock.Acquire(10 * 1000, false))
        return ~0;

    if(brk < cp->MinBrk || brk > cp->MaxBrk)
    {
        brk = cp->CurrentBrk;
        cp->MemoryLock.Release();
        return brk;
    }

    uintptr_t mappedNeeded = align(brk, PAGE_SIZE);

    if(mappedNeeded > cp->MappedBrk)
    {   // alloc and map needed memory
        for(uintptr_t va = cp->MappedBrk; va < mappedNeeded; va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::AllocPage();
            if(pa == ~0)
            {
                cp->MemoryLock.Release();;
                return cp->CurrentBrk;
            }
            if(!Paging::MapPage(cp->AddressSpace, va, pa, false, true, true))
            {
                cp->MemoryLock.Release();;
                return cp->CurrentBrk;
            }
        }
        cp->MappedBrk = mappedNeeded;
    }
    else
    {   // unmap and free excess memory
        for(uintptr_t va = mappedNeeded; va < cp->MappedBrk; va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::GetPhysicalAddress(cp->AddressSpace, va);
            if(pa != ~0)
                Paging::FreePage(pa);
            Paging::UnMapPage(cp->AddressSpace, va, false);
        }
        cp->MappedBrk = mappedNeeded;
    }

    cp->CurrentBrk = brk;
    cp->MemoryLock.Release();
    return brk;
}

long SysCalls::sys_stat(long *args) // 106
{
    if(!args[1] || !args[2]) return -EINVAL;
    const char *filename = (const char *)args[1];
    stat *st = (stat *)args[2];
    File *f = File::Open(filename, 0);
    if(!f) return -ENOENT;
    memset(st, 0, sizeof(stat));
    if(!DEntry::Lock())
    {
        delete f;
        return -EBUSY;
    }
    if(!INode::Lock())
    {
        DEntry::UnLock();
        delete f;
        return -EBUSY;
    }
    DEntry *dentry = f->DEntry;
    INode *inode = dentry->INode;
    st->st_ino = inode->Number;
    st->st_mode = inode->GetMode();
    st->st_nlink = inode->GetLinkCount();
    st->st_uid = inode->GetUID();
    st->st_gid = inode->GetGID();
    st->st_size = inode->GetSize();
    st->st_blksize = 512;
    st->st_blocks = align(st->st_size, st->st_blksize) / st->st_blksize;
    st->st_atime = inode->GetAccessTime();
    st->st_mtime = inode->GetModifyTime();
    st->st_ctime = inode->GetCreateTime();
    INode::UnLock();
    DEntry::UnLock();
    delete f;
    return 0;
}

long SysCalls::sys_fsync(long *args) // 118
{
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    File *f = cp->GetFileDescriptor(args[1]);
    if(!f) return -EBADF;
    //return f->Flush(); // Not implemented yet
    return 0;
}

long SysCalls::sys_fdatasync(long *args) // 148
{   // the same as sys_fsync for now
    return sys_fsync(args);
}

long SysCalls::sys_nanosleep(long *args) // 162
{
    struct timespec *req = (struct timespec *)args[1];
    if(!req || req->tv_sec < 0 || req->tv_nsec > 999999999)
        return -EINVAL;
    struct timespec *rem = (struct timespec *)args[2];
    uint millisReq = req->tv_sec * 1000 + req->tv_nsec / (1000 * 1000);
    uint millisLeft = Time::Sleep(millisReq, true);
    int res = 0;
    if(millisLeft)
    {
        res = -EINTR;
        if(rem)
        {
            rem->tv_sec = millisLeft / 1000;
            rem->tv_nsec = (millisLeft % 1000) * 1000 * 1000;
        }
    }
    return res;
}

long SysCalls::sys_getcwd(long *args) // 183
{
    char *buf = (char *)args[1];
    size_t size = (size_t)args[2];
    if(!buf || !size) return -EINVAL;
    DEntry *cwd = Process::GetCurrentDir();
    if(!cwd) return -ENOENT;
    size_t res = cwd->GetFullPath(buf, size);
    if(res >= (size - 1))
        return -ERANGE;
    return 0;
}

long SysCalls::sys_gettid(long *args) // 224
{
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    return cp->ID;
}

long SysCalls::sys_create_window(long *args) // 386
{
    return WindowManager::CreateWindow(args[1], args[2], args[3], args[4]);
}

long SysCalls::sys_show_window(long *args) // 387
{
    return WindowManager::ShowWindow(args[1]) ? 0 : -EINVAL;
}

long SysCalls::sys_hide_window(long *args) // 388
{
    return WindowManager::HideWindow(args[1]) ? 0 : -EINVAL;
}

long SysCalls::sys_destroy_window(long *args) // 389
{
    return WindowManager::DestroyWindow(args[1]) ? 0 : -EINVAL;
}

long SysCalls::sys_draw_rectangle(long *args) // 390
{
    WindowManager::wmRectangle *wmRect = (WindowManager::wmRectangle *)args[2];
    WindowManager::Rectangle rect(wmRect->X, wmRect->Y, wmRect->Width, wmRect->Height);
    return WindowManager::DrawRectangle(args[1], rect, args[3]);
}

long SysCalls::sys_draw_filled_rectangle(long *args) // 391
{
    WindowManager::wmRectangle *wmRect = (WindowManager::wmRectangle *)args[2];
    WindowManager::Rectangle rect(wmRect->X, wmRect->Y, wmRect->Width, wmRect->Height);
    return WindowManager::DrawFilledRectangle(args[1], rect, args[3]);
}

long SysCalls::sys_update_window(long *args) // 392
{
    return WindowManager::UpdateWindow(args[1]) ? 0 : -EINVAL;
}

long SysCalls::sys_redraw_window(long *args) // 393
{
    return WindowManager::RedrawWindow(args[1]) ? 0 : -EINVAL;
}

long SysCalls::sys_draw_line(long *args) // 394
{
    return WindowManager::DrawLine(args[1], args[2], args[3], args[4], args[5], args[6]) ? 0 : -EINVAL;
}

long SysCalls::sys_blit(long *args) // 395
{
    WindowManager::pmPixMap *src = (WindowManager::pmPixMap *)args[2];
    WindowManager::wmBlitInfo *bi = (WindowManager::wmBlitInfo *)args[3];
    PixMap::PixelFormat pf(src->Format.BPP, src->Format.AlphaShift, src->Format.RedShift, src->Format.GreenShift, src->Format.BlueShift, src->Format.AlphaBits, src->Format.RedBits, src->Format.GreenBits, src->Format.BlueBits);
    PixMap pm(src->Width, src->Height, src->Pitch, pf, src->Pixels, false);
    return WindowManager::Blit(args[1], &pm, bi->SX, bi->SY, bi->X, bi->Y, bi->Width, bi->Height) ? 0 : -EINVAL;
}

long SysCalls::sys_alpha_blit(long *args) // 396
{
    WindowManager::pmPixMap *src = (WindowManager::pmPixMap *)args[2];
    WindowManager::wmBlitInfo *bi = (WindowManager::wmBlitInfo *)args[3];
    PixMap::PixelFormat pf(src->Format.BPP, src->Format.AlphaShift, src->Format.RedShift, src->Format.GreenShift, src->Format.BlueShift, src->Format.AlphaBits, src->Format.RedBits, src->Format.GreenBits, src->Format.BlueBits);
    PixMap pm(src->Width, src->Height, src->Pitch, pf, src->Pixels, false);
    return WindowManager::AlphaBlit(args[1], &pm, bi->SX, bi->SY, bi->X, bi->Y, bi->Width, bi->Height) ? 0 : -EINVAL;
}

long SysCalls::sys_map_window(long *args) // 397
{
    if(!args[2]) return -EINVAL;
    WindowManager::Window *wnd = WindowManager::GetByID(args[1]);
    if(!wnd) return -EINVAL;
    uintptr_t pixels = (uintptr_t)wnd->Contents->Pixels;
    size_t size = (wnd->Contents->Height * wnd->Contents->Pitch) + (PAGE_SIZE - 1);
    uintptr_t va = 0x80000000;
    Process *cp = Process::GetCurrent();

    bool ok = false;
    while(va < (KERNEL_BASE - 0x10000000))
    {
        ok = true;
        for(uintptr_t addr = va, checked = 0; addr < (va + size); addr += PAGE_SIZE)
        {
            checked += PAGE_SIZE;
            uintptr_t pa = Paging::GetPhysicalAddress(cp->AddressSpace, addr);
            if(pa != ~0)
            {
                ok = false;
                va += checked;
                break;
            }
        }
        if(ok) break;
    }
    if(!ok) return -ENOMEM;

    for(uintptr_t addr = va, offs = 0; addr < (va + size); addr += PAGE_SIZE, offs += PAGE_SIZE)
    {
        uintptr_t pa = Paging::GetPhysicalAddress(cp->AddressSpace, pixels + offs);
        ok = Paging::MapPage(cp->AddressSpace, addr, pa, false, true, true);
        if(!ok)
        {
            for(uintptr_t addr = va; addr < (va + size); addr += PAGE_SIZE)
                Paging::UnMapPage(cp->AddressSpace, va, false);
            return -ENOMEM;
        }
    }

    void **ptr = (void **)args[2];
    *ptr = (void *)va;
    return 0;
}

long SysCalls::sys_invalidate_rect(long *args) // 398
{
    WindowManager::wmRectangle *wmRect = (WindowManager::wmRectangle *)args[2];
    WindowManager::Rectangle rect(wmRect->X, wmRect->Y, wmRect->Width, wmRect->Height);
    return WindowManager::InvalidateRectangle(args[1], rect) ? 0 : -EINVAL;
}

long SysCalls::sys_get_window_size(long *args) // 399
{
    WindowManager::Window *wnd = WindowManager::GetByID(args[1]);
    if(!wnd) return -EINVAL;
    int *w = (int *)args[2];
    int *h = (int *)args[3];
    if(w) *w = wnd->Contents->Width;
    if(h) *h = wnd->Contents->Height;
    return 0;
}

long SysCalls::sys_get_pixel_format(long *args) // 400
{
    WindowManager::Window *wnd = WindowManager::GetByID(args[1]);
    if(!wnd) return -EINVAL;
    PixMap::PixelFormat fmt = wnd->Contents->Format;
    WindowManager::pmPixelFormat *pmFmt = (WindowManager::pmPixelFormat *)args[2];
    int *pitch = (int *)args[3];
    if(pmFmt)
    {
        pmFmt->BPP = fmt.BPP;
        pmFmt->AlphaShift = fmt.AlphaShift;
        pmFmt->RedShift = fmt.RedShift;
        pmFmt->GreenShift = fmt.GreenShift;
        pmFmt->BlueShift = fmt.BlueShift;
        pmFmt->AlphaBits = fmt.AlphaBits;
        pmFmt->RedBits = fmt.RedBits;
        pmFmt->GreenBits = fmt.GreenBits;
        pmFmt->BlueBits = fmt.BlueBits;
    }
    if(pitch) *pitch = wnd->Contents->Pitch;
    return 0;
}

void SysCalls::Initialize()
{
    Ints::RegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

void SysCalls::Cleanup()
{
    Ints::UnRegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

