#include <errno.h>
#include <sys/times.h>

clock_t times(struct tms *buf)
{
    errno = EINVAL;
    return (clock_t)-1;
}

