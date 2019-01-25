#ifndef TIMES_H
#define TIMES_H

#include <time.h>

struct tms
{
    clock_t tms_utime;  /* user time */
    clock_t tms_stime;  /* system time */
    clock_t tms_cutime; /* user time of children */
    clock_t tms_cstime; /* system time of children */
};

clock_t times(struct tms *buf);

#endif // TIMES_H
