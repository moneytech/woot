#include <stdint.h>
#include <sys/syscall.h>

#include "internal/syscall.h"

extern uintptr_t __current_brk;

void __init_libc()
{
    __current_brk = (uintptr_t)syscall1(SYS_brk, 0);
}
