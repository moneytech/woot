#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "internal/syscall.h"

int open(const char *pathname, int flags, ...)
{
    va_list arg;
    va_start(arg, flags);
    mode_t mode = flags & O_CREAT ? va_arg(arg, mode_t) : 0;
    va_end();
    long res = syscall3(SYS_open, (long)pathname, flags, mode);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}
