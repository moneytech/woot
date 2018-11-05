#include <errno.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "internal/syscall.h"

int stat(const char *path, struct stat *buf)
{
    long res = syscall2(SYS_stat, (long)path, (long)buf);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

int mkdir(const char *pathname, mode_t mode)
{
    long res = syscall2(SYS_mkdir, (long)pathname, (long)mode);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}
