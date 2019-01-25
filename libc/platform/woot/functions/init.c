#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h>

extern uintptr_t __current_brk;

void __init_libc()
{
    __current_brk = (uintptr_t)syscall1(SYS_brk, 0);
}

void __stack_chk_fail_local()
{
    abort();
}

void __stack_chk_fail()
{
    abort();
}
