#include <sys/syscall.h>
#include <time.h>

#include "internal/syscall.h"

static int isLeap(int year)
{
    if(year % 4) return 0;
    else if(year % 100) return 1;
    else if(year % 400) return 0;
    return 1;
}

static time_t tmToUnix(struct tm *tm)
{
    if(!tm) return 0;

    time_t t;
    time_t y = tm->tm_year;
    time_t m = tm->tm_mon;
    time_t d = tm->tm_mday;

    if(m <= 2)
    {
        m += 12;
        y -= 1;
    }

    t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
    t += (30 * m) + (3 * (m + 1) / 5) + d;
    t -= 719561;
    t *= 86400;
    t += (3600 * tm->tm_hour) + (60 * tm->tm_min) + tm->tm_sec;

    return t;
}

static void unixToTM(time_t t, struct tm *tm)
{
    static const int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if(!tm) return;
    if(t < 0) t = 0; // negative time not supported
    tm->tm_sec = t % 60;
    t /= 60;
    tm->tm_min = t % 60;
    t /= 60;
    tm->tm_hour = t % 24;
    t /= 24;

    time_t a = (4 * t + 102032) / 146097 + 15;
    time_t b = t + 2442113 + a - (a / 4);
    time_t year = (20 * b - 2442) / 7305;
    time_t d = b - 365 * year - (year / 4);
    time_t month = d * 1000 / 30601;
    time_t day = d - month * 30 - month * 601 / 1000;

    if(month <= 13)
    {
        year -= 4716;
        month -= 1;
    }
    else
    {
        year -= 4715;
        month -= 13;
    }

    tm->tm_year = year;
    tm->tm_mon = month;
    tm->tm_mday = day;

    int yday = 0;
    int leap = isLeap(year);
    for(int i = 0; i < month; ++i)
    {
        yday += monthDays[i];
        if(leap && i == 2)
            ++yday;
    }
    yday += day;
    tm->tm_yday = yday;
    tm->tm_wday = (year * 365 + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400) % 7;
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    return syscall2(SYS_nanosleep, (long)req, (long)rem);
}

time_t time(time_t *t)
{
    return syscall1(SYS_time, (long)t);
}

struct tm *localtime(const time_t *timer)
{
    static struct tm result;
    unixToTM(*timer, &result);
    return &result;
}

time_t mktime(struct tm *timeptr)
{
    return tmToUnix(timeptr);
}
