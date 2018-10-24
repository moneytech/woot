#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <ints.h>
#include <types.h>

#define MAX_SYSCALLS 512

class SysCalls
{
    typedef long (*Callback)(long *args);

    static Ints::Handler handler;
    static Callback callbacks[MAX_SYSCALLS];
    static bool isr(Ints::State *state, void *context);

    static long sys_exit(long *args); // 1
    static long sys_write(long *args); // 4
    static long sys_getpid(long *args); // 20
    static long sys_nanosleep(long *args); // 162
    static long sys_gettid(long *args); // 224
public:
    static void Initialize();
    static void Cleanup();
};

#endif // SYSCALLS_H
