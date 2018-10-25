#include <sys/syscall.h>
#include <time.h>

#include "internal/syscall.h"

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    return syscall2(SYS_nanosleep, (long)req, (long)rem);
}

time_t time(time_t *t)
{
    return syscall1(SYS_time, (long)t);
}
