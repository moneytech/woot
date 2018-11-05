#include <cpu.h>
#include <debugstream.h>
#include <dentry.h>
#include <errno.h>
#include <file.h>
#include <inode.h>
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
SysCalls::Callback SysCalls::callbacks[] =
{
    [SYS_exit] = sys_exit,
    [SYS_read] = sys_read,
    [SYS_write] = sys_write,
    [SYS_open] = sys_open,
    [SYS_close] = sys_close,
    [SYS_time] = sys_time,
    [SYS_lseek] = sys_lseek,
    [SYS_getpid] = sys_getpid,
    [SYS_mkdir] = sys_mkdir,
    [SYS_brk] = sys_brk,
    [SYS_stat] = sys_stat,
    [SYS_fsync] = sys_fsync,
    [SYS_nanosleep] = sys_nanosleep,
    [SYS_getcwd] = sys_getcwd,
    [SYS_gettid] = sys_gettid,
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
    return f->Seek(args[1], args[2]);
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

void SysCalls::Initialize()
{
    Ints::RegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

void SysCalls::Cleanup()
{
    Ints::UnRegisterHandler(SYSCALLS_INT_VECTOR, &handler);
}

