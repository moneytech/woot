#include <stdlib.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "internal/syscall.h"

int errno = 0;

extern int main(int argc, char *argv[]);
extern uintptr_t __current_brk;

void _start()
{
    __current_brk = (uintptr_t)syscall1(SYS_brk, 0);
    char *arg0 = "test";
    char *argv[] = { arg0 };
    exit(main(1, argv));
}

void __stack_chk_fail_local()
{
    abort();
}
