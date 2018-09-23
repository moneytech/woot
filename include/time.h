#ifndef TIME_H
#define TIME_H

#include <types.h>

#ifdef __cplusplus
#include <ints.h>

class Time
{
    static uint64_t systemTicks;
    static uint64_t systemTickFreq;
public:
    static bool isFakeTick; // TODO: make this private
private:
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
    static uint Sleep(uint millis, bool interruptible);
};

extern "C" {
#endif // __cplusplus

time_t time(time_t *tloc);
int gettimeofday(time_t *t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TIME_H
