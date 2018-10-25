#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "internal/syscall.h"

extern void *end;
uintptr_t __current_brk = (uintptr_t)&end;

void _exit(int status)
{
    for(;;)
        syscall1(SYS_exit, status);
}

int open(const char *pathname, int flags)
{
    return -ENOSYS;
}

ssize_t read(int fd, void *buf, size_t count)
{
    return -ENOSYS;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    return syscall3(SYS_write, fd, (long)buf, count);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return -ENOSYS;
}

int close(int fd)
{
    return -ENOSYS;
}

pid_t getpid()
{
    return syscall0(SYS_getpid);
}

int brk(void *addr)
{
    int res = syscall1(SYS_brk, (long)addr);
    if(res < 0)
    {
        errno = -res;
        res = -1;
    }
    __current_brk = (uintptr_t)addr;
    return res;
}

void *sbrk(intptr_t increment)
{
    uintptr_t cbrk = __current_brk;
    int res = brk((void *)(cbrk + increment));
    if(res < 0) return (void *)-1;
    return (void *)cbrk;
}
