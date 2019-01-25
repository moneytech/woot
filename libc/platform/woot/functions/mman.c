#include <errno.h>
#include <sys/mman.h>
#include <sys/syscall.h>

int munmap(void *addr, size_t length)
{
    long res = syscall2(SYS_munmap, (long)addr, length);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}
