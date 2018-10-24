#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "internal/syscall.h"

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
