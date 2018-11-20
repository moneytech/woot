#include <errno.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "internal/syscall.h"

extern void *end;
uintptr_t __current_brk = (uintptr_t)&end;

int access(const char *pathname, int mode)
{
    return 0;
}

void _exit(int status)
{
    for(;;)
        syscall1(SYS_exit, status);
}

ssize_t read(int fd, void *buf, size_t count)
{
    return syscall3(SYS_read, fd, (long)buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    return syscall3(SYS_write, fd, (long)buf, count);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return syscall3(SYS_lseek, fd, offset, whence);
}

int close(int fd)
{
    return syscall1(SYS_close, fd);
}

pid_t getpid()
{
    return syscall0(SYS_getpid);
}

int brk(void *addr)
{
    uintptr_t res = syscall1(SYS_brk, (long)addr);
    if(res == ~0)
    {
        errno = ENOMEM;
        return -1;
    }
    __current_brk = (uintptr_t)addr;
    return res;
}

void *sbrk(intptr_t increment)
{
    uintptr_t cbrk = __current_brk;
    uintptr_t res = syscall1(SYS_brk, (long)(cbrk + increment));
    if(res == ~0) return (void *)(-1);
    __current_brk = res;
    return (void *)cbrk;
}

char *getcwd(char *buf, size_t size)
{
    long res = syscall2(SYS_getcwd, (long)buf, size);
    if(res < 0)
    {
        errno = -res;
        return NULL;
    }
    return buf;
}

char *getwd(char *buf)
{
    return getcwd(buf, SIZE_MAX);
}

char *get_current_dir_name(void)
{
    size_t size = 64;
    char *buf = (char *)malloc(size);
    if(!buf)
    {
        errno = ENOMEM;
        return NULL;
    }
    while(!getcwd(buf, size))
    {
        if(errno != ERANGE)
        {
            free(buf);
            return NULL;
        }
        size *= 2;
        if(size >= 8192)
        {
            free(buf);
            errno = ENAMETOOLONG;
            return NULL;
        }
        buf = (char *)realloc(buf, size);
        if(!buf)
        {
            errno = ENOMEM;
            return NULL;
        }
    }
    return buf;
}

int fsync(int fd)
{
    long res = syscall1(SYS_fsync, fd);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

int fdatasync(int fd)
{
    long res = syscall1(SYS_fdatasync, fd);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

int unlink(const char *pathname)
{
    long res = syscall1(SYS_unlink, (long)pathname);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}
