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
    static long sys_read(long *args); // 3;
    static long sys_write(long *args); // 4
    static long sys_open(long *args); // 5
    static long sys_close(long *args); // 6
    static long sys_time(long * args); // 13
    static long sys_lseek(long *args); // 19
    static long sys_getpid(long *args); // 20
    static long sys_brk(long *args); // 45
    static long sys_fsync(long *args); // 118
    static long sys_fdatasync(long *args); // 148
    static long sys_nanosleep(long *args); // 162
    static long sys_getcwd(long *args); // 183
    static long sys_gettid(long *args); // 224
public:
    static void Initialize();
    static void Cleanup();
};

#endif // SYSCALLS_H
