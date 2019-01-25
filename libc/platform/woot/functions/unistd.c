#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

extern void *end;
uintptr_t __current_brk = (uintptr_t)&end;

char *_environ[] =
{
    "NOT_IMPLEMENTED=1",
    NULL
};

char **environ = _environ;

int access(const char *pathname, int mode)
{
    return 0;
}

int getpagesize(void)
{
    return 4096;
}

void _exit(int status)
{
    for(;;)
        syscall1(SYS_exit, status);
}

ssize_t read(int fd, void *buf, size_t count)
{
    int res = syscall3(SYS_read, fd, (long)buf, count);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    int res = syscall3(SYS_write, fd, (long)buf, count);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

off_t lseek(int fd, off_t offset, int whence)
{
    off_t res = syscall3(SYS_lseek, fd, offset, whence);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

off64_t lseek64(int fd, off64_t offset, int whence)
{
    off64_t result;
    long res = syscall5(SYS__llseek, fd, (offset >> 32) & 0xFFFFFFFF, offset & 0xFFFFFFFF, (long)&result, whence);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return result;
}

int close(int fd)
{
    int res = syscall1(SYS_close, fd);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

pid_t getpid()
{
    return syscall0(SYS_getpid);
}

int brk(void *addr)
{
    uintptr_t res = syscall1(SYS_brk, (long)addr);
    if(res == ~0u)
    {
        errno = ENOMEM;
        return -1;
    }
    __current_brk = (uintptr_t)addr;
    return 0;
}

void *sbrk(intptr_t increment)
{
    uintptr_t cbrk = __current_brk;
    uintptr_t res = syscall1(SYS_brk, (long)(cbrk + increment));
    if(res == ~0u) return (void *)(-1);
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

int execve(const char *filename, char *const argv[], char *const envp[])
{
    fprintf(stderr, "execve not implemented\n");
    errno = ENOSYS;
    return -1;
}

pid_t fork()
{
    fprintf(stderr, "fork not implemented\n");
    errno = ENOSYS;
    return -1;
}

pid_t wait(int *status)
{
    fprintf(stderr, "wait not implemented\n");
    errno = ENOSYS;
    return -1;
}

int link(const char *oldpath, const char *newpath)
{
    fprintf(stderr, "link not implemented\n");
    errno = ENOSYS;
    return -1;
}
