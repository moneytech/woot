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
    static long sys_msleep(long *args);
public:
    static void Initialize();
    static void Cleanup();
};

#endif // SYSCALLS_H
