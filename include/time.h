#ifndef TIME_H
#define TIME_H

#include <ints.h>
#include <types.h>

class Time
{
    static uint64_t systemTicks;
    static uint64_t systemTickFreq;
    static bool isFakeTick;
    static Ints::Handler tickHandler;
    static bool Tick(Ints::State *state, void *context);
public:

    struct DateTime
    {
        int Year;
        int Month;
        int Day;
        int Hour;
        int Minute;
        int Second;
        int Millisecond;
    };

    static void Initialize();
    static void StartSystemTimer();
    static void GetDateTime(DateTime *dt);
    static void UnixToDateTime(time_t t, DateTime *date);
    static void FracUnixToDateTime(double t, DateTime *date);
    static time_t DateTimeToUnix(const DateTime *date);
    static uint64_t GetTickFrequency();
    static uint64_t GetTickCount();
    static double GetSystemUpTime();
    static void FakeTick();
};

time_t time(time_t *tloc);
int gettimeofday(time_t *t);


#endif // TIME_H
