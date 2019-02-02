#include <sys/syscall.h>
#include <woot/time.h>

int tmSleep(int ms)
{
    return syscall1(SYS_sleep_ms, ms);
}

unsigned long long tmGetTicks()
{
    unsigned long long v;
    syscall1(SYS_get_ticks, (long)&v);
    return v;
}

unsigned long long tmGetTickFreq()
{
    unsigned long long v;
    syscall1(SYS_get_tick_freq, (long)&v);
    return v;
}
