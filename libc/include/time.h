#ifndef TIME_H
#define TIME_H

typedef long time_t;

struct timespec
{
    time_t tv_sec;
    long tv_nsec;
};

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

int nanosleep(const struct timespec *req, struct timespec *rem);
time_t time(time_t *t);
struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer); // not thread safe
time_t mktime(struct tm *timeptr);

#endif // TIME_H
