#ifndef TIME_H
#define TIME_H

typedef long long time_t;

struct timespec
{
    time_t tv_sec;
    long tv_nsec;
};

int nanosleep(const struct timespec *req, struct timespec *rem);

#endif // TIME_H
