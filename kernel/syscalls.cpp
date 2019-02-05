#include <audiodevice.h>
#include <cpu.h>
#include <debugstream.h>
#include <dentry.h>
#include <directoryentry.h>
#include <errno.h>
#include <file.h>
#include <filesystem.h>
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

struct timespec
{
    time_t tv_sec;
    long tv_nsec;
};

#define NAME_MAX 255

#define DT_UNKNOWN 0
#define DT_REG 1
#define DT_DIR 2
#define DT_FIFO 3
#define DT_SOCK 4
#define DT_CHR 5
#define DT_BLK 6
#define DT_LNK 7

struct dirent
{
    int d_ino;
    unsigned char d_type;
    char d_name[NAME_MAX + 1];
};

#define ET_INVALID  0
#define ET_OTHER    1
#define ET_KEYBOARD 2
#define ET_MOUSE    3

static void eventTowmEvent(WindowManager::wmEvent *event, InputDevice::Event *ev, WindowManager::Rectangle *rect)
{
    WindowManager::Point mousePoint;
    if(ev->DeviceType == InputDevice::Type::Mouse)
        mousePoint = WindowManager::GetMousePosition();
    switch(ev->DeviceType)
    {
    case InputDevice::Type::Unknown:
        event->Type = ET_INVALID;
        break;
    default:
        event->Type = ET_OTHER;
        break;
    case InputDevice::Type::Keyboard:
        event->Type = ET_KEYBOARD;
        event->Keyboard.Key = (int)ev->Keyboard.Key;
        event->Keyboard.Flags |= ev->Keyboard.Release ? 1 : 0;
        break;
    case InputDevice::Type::Mouse:
        event->Type = ET_MOUSE;
        event->Mouse.X = mousePoint.X - rect->Origin.X;
        event->Mouse.Y = mousePoint.Y - rect->Origin.Y;
        event->Mouse.DeltaX = ev->Mouse.Movement[0];
        event->Mouse.DeltaY = ev->Mouse.Movement[1];
        event->Mouse.ButtonsPressed = ev->Mouse.ButtonsPressed;
        event->Mouse.ButtonsReleased = ev->Mouse.ButtonsReleased;
        event->Mouse.ButtonsHeld = ev->Mouse.ButtonsHeld;
        break;
    }
}

extern DebugStream debugStream;

Ints::Handler SysCalls::handler = { nullptr, SysCalls::isr, nullptr };

// syscall table
SysCalls::Callback SysCalls::callbacks[] =
{
    nullptr, sys_exit, nullptr, sys_read, sys_write, sys_open, sys_close, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_time, nullptr, nullptr, // 0 - 15
    nullptr, nullptr, nullptr, sys_lseek, sys_getpid, nullptr, nullptr, sys_setuid, sys_getuid, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 16 - 31
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_mkdir, nullptr, nullptr, nullptr, nullptr, nullptr, sys_brk, sys_setgid, sys_getgid, // 32 - 47
    nullptr, sys_geteuid, sys_getegid, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 48 - 63
    sys_getppid, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 64 - 79
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_readdir, nullptr, sys_munmap, nullptr, nullptr, nullptr, nullptr, // 80 - 95
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_stat, nullptr, sys_fstat, nullptr, nullptr, nullptr, // 96 - 111
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_fsync, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 112 - 127
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_llseek, nullptr, nullptr, nullptr, // 128 - 143
    nullptr, nullptr, nullptr, nullptr, sys_fdatasync, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 144 - 159
    nullptr, nullptr, sys_nanosleep, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 160 - 175
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_getcwd, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 176 - 191
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, sys_getuid32, sys_getgid32, sys_geteuid32, sys_getegid32, nullptr, nullptr, nullptr, nullptr, nullptr, // 192 - 207
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
    sys_get_pixel_format, sys_set_drag_rect, sys_get_event, sys_peek_event, sys_audio_open, sys_audio_close, sys_audio_get_frame_size, sys_audio_write, sys_audio_start_playback, sys_audio_stop_playback, sys_audio_get_buffer_count, sys_audio_get_device_vendor, sys_audio_get_device_model, nullptr, nullptr, nullptr, // 400 - 415
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 416 - 431
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 432 - 447
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 448 - 463
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 464 - 479
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 480 - 495
    nullptr, nullptr, nullptr, nullptr, sys_redraw_screen, sys_sleep_ms, sys_get_ticks, sys_get_tick_freq, sys_thread_create, sys_thread_delete, sys_thread_suspend, sys_thread_resume, sys_thread_sleep, sys_mutex_create, sys_mutex_delete, sys_mutex_acquire,  // 496 - 511

    sys_mutex_release, sys_mutex_cancel, sys_semaphore_create, sys_semaphore_delete, sys_semaphore_wait, sys_semaphore_signal, sys_semaphore_reset, sys_semaphore_cancel, sys_semaphore_get_count, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 512 - 527
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 528 - 543
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 544 - 559
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 560 - 575
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 576 - 591
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 592 - 607
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 608 - 623
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 624 - 639
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 640 - 655
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 656 - 671
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 672 - 687
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 688 - 703
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 704 - 719
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 720 - 735
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 736 - 751
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr  // 752 - 767
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
    return cp->Close(args[1]);
}

long SysCalls::sys_time(long *args) // 13
{
    return time((time_t *)args[1]);
}

long SysCalls::sys_lseek(long *args) // 19
{
    Process *cp = Process::GetCurrent();
    File *f = cp->GetFileDescriptor(args[1]);
    if(!f) return -EBADF;
    return f->Seek(args[2], args[3]);
}

long SysCalls::sys_getpid(long *args) // 20
{
    return Process::GetCurrent()->ID;
}

long SysCalls::sys_setuid(long *args) // 23
{
    Process *cp = Process::GetCurrent();
    // TODO: add permission checking here
    cp->EUID = args[1];
    return 0;
}

long SysCalls::sys_getuid(long *args)
{
    Process *cp = Process::GetCurrent();
    return cp->UID;
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

long SysCalls::sys_setgid(long *args) // 46
{
    Process *cp = Process::GetCurrent();
    // TODO: add permission checking here
    cp->EGID = args[1];
    return 0;
}

long SysCalls::sys_getgid(long *args) // 47
{
    Process *cp = Process::GetCurrent();
    return cp->GID;
}

long SysCalls::sys_geteuid(long *args) // 49
{
    Process *cp = Process::GetCurrent();
    return cp->EUID;
}

long SysCalls::sys_getegid(long *args) // 50
{
    Process *cp = Process::GetCurrent();
    return cp->EGID;
}

long SysCalls::sys_getppid(long *args) // 64
{
    Process *cp = Process::GetCurrent();
    Process *pp = cp->Parent;
    return pp ? pp->ID : -ESRCH;
}

long SysCalls::sys_readdir(long *args) // 89
{
    if(!args[2] || args[3] != 1)
        return -EINVAL;
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    File *f = cp->GetFileDescriptor(args[1]);
    if(!f) return -EBADF;
    struct dirent *dent = (struct dirent *)args[2];
    dent->d_ino = -1;
    dent->d_type = DT_UNKNOWN;
    dent->d_name[0] = 0;
    DirectoryEntry *de = f->ReadDir();
    if(!de) return 0;
    dent->d_ino = de->INode;
    dent->d_type = S_ISDIR(de->Mode) ? DT_DIR : DT_REG;
    strncpy(dent->d_name, de->Name, sizeof(dent->d_name));
    delete de;
    return 0;
}

long SysCalls::sys_munmap(long *args) // 91
{
    uintptr_t addr = (uintptr_t)args[1];
    size_t n = (size_t)args[2] / PAGE_SIZE;
    if(addr % PAGE_SIZE || args[2] % PAGE_SIZE || addr >= KERNEL_BASE || (addr + n * PAGE_SIZE) >= KERNEL_BASE)
        return -EINVAL;
    Process *cp = Process::GetCurrent();
    bool res = Paging::UnMapPages(cp->AddressSpace, addr, false, n);
    return res ? 0 : -EINVAL;
}

long SysCalls::sys_stat(long *args) // 106
{
    if(!args[1] || !args[2]) return -EINVAL;
    const char *filename = (const char *)args[1];
    stat *st = (stat *)args[2];
    File *f = File::Open(filename, 0);
    if(!f) return -ENOENT;
    memset(st, 0, sizeof(stat));
    if(!FileSystem::GlobalLock())
    {
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
    FileSystem::GlobalUnLock();
    delete f;
    return 0;
}

long SysCalls::sys_fstat(long *args) // 108
{
    int fd = args[1];
    stat *st = (stat *)args[2];
    if(!st) return -EINVAL;
    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    File *f = cp->GetFileDescriptor(fd);
    if(!f) return -EBADF;
    memset(st, 0, sizeof(stat));
    if(!FileSystem::GlobalLock())
        return -EBUSY;
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
    FileSystem::GlobalUnLock();
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

long SysCalls::sys_llseek(long *args) // 140
{
    int fd = args[1];
    uint64_t offsH = args[2];
    uint64_t offsL = args[3];
    int64_t *result = (int64_t *)(uintptr_t)args[4];
    int whence = args[5];

    Process *cp = Process::GetCurrent();
    if(!cp) return -ESRCH;
    File *f = cp->GetFileDescriptor(fd);
    if(!f) return -EBADF;
    int64_t res = f->Seek(offsH << 32 | offsL, whence);
    if(result) *result = res;
    if(res < 0) return res;
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

long SysCalls::sys_getuid32(long *args) // 199
{
    return sys_getuid(args);
}

long SysCalls::sys_getgid32(long *args) // 200
{
    return sys_getgid(args);
}

long SysCalls::sys_geteuid32(long *args) // 201
{
    return sys_geteuid(args);
}

long SysCalls::sys_getegid32(long *args) // 202
{
    return sys_getegid(args);
}

long SysCalls::sys_gettid(long *args) // 224
{
    Thread *ct = Thread::GetCurrent();
    if(!ct) return -ESRCH;
    return ct->ID;
}

long SysCalls::sys_create_window(long *args) // 386
{
    return WindowManager::CreateWindow(args[1], args[2], args[3], args[4], nullptr);
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
    PixMap pm(src->Contents.Width, src->Contents.Height, src->Pitch, pf, src->Pixels, false);
    return WindowManager::Blit(args[1], &pm, bi->SX, bi->SY, bi->X, bi->Y, bi->Width, bi->Height) ? 0 : -EINVAL;
}

long SysCalls::sys_alpha_blit(long *args) // 396
{
    WindowManager::pmPixMap *src = (WindowManager::pmPixMap *)args[2];
    WindowManager::wmBlitInfo *bi = (WindowManager::wmBlitInfo *)args[3];
    PixMap::PixelFormat pf(src->Format.BPP, src->Format.AlphaShift, src->Format.RedShift, src->Format.GreenShift, src->Format.BlueShift, src->Format.AlphaBits, src->Format.RedBits, src->Format.GreenBits, src->Format.BlueBits);
    PixMap pm(src->Contents.Width, src->Contents.Height, src->Pitch, pf, src->Pixels, false);
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

long SysCalls::sys_set_drag_rect(long *args) // 401
{
    WindowManager::wmRectangle *wmRect = (WindowManager::wmRectangle *)args[2];
    WindowManager::Rectangle rect(wmRect->X, wmRect->Y, wmRect->Width, wmRect->Height);
    return WindowManager::SetDragRectangle(args[1], rect) ? 0 : -EINVAL;
}

long SysCalls::sys_get_event(long *args) // 402
{
    WindowManager::wmEvent *event = (WindowManager::wmEvent *)args[2];
    if(!event) return -EINVAL;
    memset(event, 0, sizeof(WindowManager::wmEvent));
    WindowManager::Rectangle rect = WindowManager::GetWindowRectangle(args[1]);
    InputDevice::Event ev = WindowManager::GetEvent_nolock(args[1]); // may cause problems
    eventTowmEvent(event, &ev, &rect);
    return 1;
}

long SysCalls::sys_peek_event(long *args) // 403
{
    WindowManager::wmEvent *event = (WindowManager::wmEvent *)args[2];
    if(!event) return -EINVAL;
    memset(event, 0, sizeof(WindowManager::wmEvent));
    WindowManager::Rectangle rect = WindowManager::GetWindowRectangle(args[1]);
    InputDevice::Event ev = WindowManager::PeekEvent(args[1], args[3]);
    if(ev.DeviceType == InputDevice::Type::Unknown)
        return 0;
    eventTowmEvent(event, &ev, &rect);
    return 1;
}

long SysCalls::sys_audio_open(long *args) // 404
{
    int id = args[1];
    int rate = args[2];
    int channels = args[3];
    int bits = args[4];
    int samples = args[5];

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    res = dev->Open(rate, channels, bits, samples);
    AudioDevice::UnLock();
    return res;
}

long SysCalls::sys_audio_close(long *args) // 405
{
    int id = args[1];

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    dev->Close();
    AudioDevice::UnLock();
    return 0;
}

long SysCalls::sys_audio_get_frame_size(long *args) // 406
{
    int id = args[1];

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    res = dev->GetFrameSize();
    AudioDevice::UnLock();
    return res;
}

long SysCalls::sys_audio_write(long *args) // 407
{
    int id = args[1];
    void *ptr = (void *)(uintptr_t)args[2];

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    res = dev->Write(ptr);
    AudioDevice::UnLock();
    return res;
}

long SysCalls::sys_audio_start_playback(long *args) // 408
{
    int id = args[1];

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    res = dev->Start();
    AudioDevice::UnLock();
    return res;
}

long SysCalls::sys_audio_stop_playback(long *args) // 409
{
    int id = args[1];

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    res = dev->Stop();
    AudioDevice::UnLock();
    return res;
}

long SysCalls::sys_audio_get_buffer_count(long *args) // 410
{
    int id = args[1];

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    res = dev->GetBufferCount();
    AudioDevice::UnLock();
    return res;
}

long SysCalls::sys_audio_get_device_vendor(long *args) // 411
{
    int id = args[1];
    char *buffer = (char *)args[2];
    int bufSize = args[3];
    if(!buffer || bufSize <= 0)
        return -EINVAL;
    buffer[0] = 0;

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    const char *str = dev->GetVendor();
    strncpy(buffer, str, bufSize);
    buffer[bufSize - 1] = 0;
    AudioDevice::UnLock();

    return 0;
}

long SysCalls::sys_audio_get_device_model(long *args) // 412
{
    int id = args[1];
    char *buffer = (char *)args[2];
    int bufSize = args[3];
    if(!buffer || bufSize <= 0)
        return -EINVAL;
    buffer[0] = 0;

    int res = AudioDevice::Lock();
    if(res) return res;
    AudioDevice *dev = AudioDevice::GetByID_nolock(id);
    if(!dev)
    {
        AudioDevice::UnLock();
        return -EINVAL;
    }
    const char *str = dev->GetModel();
    strncpy(buffer, str, bufSize);
    buffer[bufSize - 1] = 0;
    AudioDevice::UnLock();

    return 0;
}

long SysCalls::sys_redraw_screen(long *args) // 500
{
    WindowManager::RedrawWindow(0);
}

long SysCalls::sys_sleep_ms(long *args) // 501
{
    int ms = args[1];
    if(ms < 0) return -EINVAL;
    return Time::Sleep(ms, false);
}

long SysCalls::sys_get_ticks(long *args) // 501
{
    uint64_t *r = (uint64_t *)args[1];
    if(!r) return -EINVAL;
    *r = Time::GetTickCount();
    return 0;
}

long SysCalls::sys_get_tick_freq(long *args) // 503
{
    uint64_t *r = (uint64_t *)args[1];
    if(!r) return -EINVAL;
    *r = Time::GetTickFrequency();
    return 0;
}

long SysCalls::sys_thread_create(long *args) // 504
{
    void *entry = (void *)args[1];
    int semId = args[2];
    int *retVal = (int *)args[3];
    void *arg = (void *)args[4];
    if(!entry) return -EINVAL;
    Process *cp = Process::GetCurrent();
    Semaphore *finished = cp->GetSemaphore(semId);
    Thread *thread = new Thread("Thread", nullptr, entry, (uintptr_t)arg, DEFAULT_STACK_SIZE, DEFAULT_STACK_SIZE, retVal, finished, true);
    thread->Enable();
    return thread->ID;
}

long SysCalls::sys_thread_delete(long *args) // 505
{
    int id = args[1];
    int retVal = args[2];
    Thread *ct = Thread::GetCurrent();
    Thread *thread = Thread::GetByID(id);
    if(!thread) return -ESRCH;
    if(thread->Process != ct->Process)
        return -EINVAL;
    Thread::Finalize(thread, retVal);
    return 0;
}

long SysCalls::sys_thread_suspend(long *args) // 506
{
    int id = args[1];
    Thread *ct = Thread::GetCurrent();
    Thread *thread = Thread::GetByID(id);
    if(!thread) return -ESRCH;
    if(thread->Process != ct->Process)
        return -EINVAL;
    thread->Suspend();
    return 0;
}

long SysCalls::sys_thread_resume(long *args) // 507
{
    int id = args[1];
    Thread *ct = Thread::GetCurrent();
    Thread *thread = Thread::GetByID(id);
    if(!thread) return -ESRCH;
    if(thread->Process != ct->Process)
        return -EINVAL;
    thread->Resume(false);
    return 0;
}

long SysCalls::sys_thread_sleep(long *args) // 508
{
    int id = args[1];
    int ms = args[2];
    bool interruptible = false;
    if(ms < 0)
    {
        interruptible = true;
        ms = -ms;
    }
    Thread *ct = Thread::GetCurrent();
    Thread *thread = Thread::GetByID(id);
    if(!thread) return -ESRCH;
    if(thread->Process != ct->Process)
        return -EINVAL;
    return thread->Sleep(ms, interruptible);
}

long SysCalls::sys_mutex_create(long *args) // 509
{
    Process *cp = Process::GetCurrent();
    return cp->NewMutex();
}

long SysCalls::sys_mutex_delete(long *args) // 510
{
    int id = args[1];
    Process *cp = Process::GetCurrent();
    return cp->DeleteMutex(id);
}

long SysCalls::sys_mutex_acquire(long *args) // 511
{
    int id = args[1];
    int timeout = args[2];
    bool tryAcquire = args[3] != 0;
    Process *cp = Process::GetCurrent();
    Mutex *mtx = cp->GetMutex(id);
    if(!mtx) return -EINVAL;
    return mtx->Acquire(timeout, tryAcquire) ? 0 : -EBUSY;
}

long SysCalls::sys_mutex_release(long *args) // 512
{
    int id = args[1];
    Process *cp = Process::GetCurrent();
    Mutex *mtx = cp->GetMutex(id);
    if(!mtx) return -EINVAL;
    mtx->Release();
    return 0;
}

long SysCalls::sys_mutex_cancel(long *args) // 513
{
    int id = args[1];
    pid_t threadId = args[2];
    Process *cp = Process::GetCurrent();
    Mutex *mtx = cp->GetMutex(id);
    if(!mtx) return -EINVAL;
    Thread *thread = Thread::GetByID(threadId);
    if(!thread) return -EINVAL;
    mtx->Cancel(thread);
    return 0;
}

long SysCalls::sys_semaphore_create(long *args) // 514
{
    int initVal = args[1];
    Process *cp = Process::GetCurrent();
    return cp->NewSemaphore(initVal);
}

long SysCalls::sys_semaphore_delete(long *args) // 515
{
    int id = args[1];
    Process *cp = Process::GetCurrent();
    return cp->DeleteSemaphore(id);
}

long SysCalls::sys_semaphore_wait(long *args) // 516
{
    int id = args[1];
    int timeout = args[2];
    bool tryWait = args[3] != 0;
    Process *cp = Process::GetCurrent();
    Semaphore *sem = cp->GetSemaphore(id);
    if(!sem) return -EINVAL;
    return sem->Wait(timeout, tryWait, false) ? 0 : -EBUSY;
}

long SysCalls::sys_semaphore_signal(long *args) // 517
{
    int id = args[1];
    Process *cp = Process::GetCurrent();
    Semaphore *sem = cp->GetSemaphore(id);
    if(!sem) return -EINVAL;
    sem->Signal(nullptr);
    return 0;
}

long SysCalls::sys_semaphore_reset(long *args) // 518
{
    int id = args[1];
    int value = args[2];
    Process *cp = Process::GetCurrent();
    Semaphore *sem = cp->GetSemaphore(id);
    if(!sem) return -EINVAL;
    sem->Reset(value);
    return 0;
}

long SysCalls::sys_semaphore_cancel(long *args) // 519
{
    int id = args[1];
    pid_t threadId = args[2];
    Process *cp = Process::GetCurrent();
    Semaphore *sem = cp->GetSemaphore(id);
    if(!sem) return -EINVAL;
    Thread *thread = Thread::GetByID(threadId);
    if(!thread) return -EINVAL;
    sem->Cancel(thread);
    return 0;
}

long SysCalls::sys_semaphore_get_count(long *args) // 520
{
    int id = args[1];
    Process *cp = Process::GetCurrent();
    Semaphore *sem = cp->GetSemaphore(id);
    if(!sem) return -EINVAL;
    return sem->GetCount();
}

void SysCalls::Initialize()
{
    Ints::RegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

void SysCalls::Cleanup()
{
    Ints::UnRegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

