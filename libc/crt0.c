#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

extern int main(int argc, char *argv[]);
extern uintptr_t __current_brk;

#define BRK_ALIGN 65536

void _start()
{
    brk((void *)(BRK_ALIGN * ((__current_brk + (BRK_ALIGN - 1)) / BRK_ALIGN)));
    char *arg0 = "test";
    char *argv[] = { arg0 };
    exit(main(1, argv));
}

void __stack_chk_fail_local()
{
    abort();
}
