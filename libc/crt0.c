#include <stdlib.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "internal/syscall.h"

int errno = 0;

extern int main(int argc, char *argv[], char *envp[]);

extern uintptr_t __current_brk;

asm(
".globl _start\n"
"_start:\n"
"fninit\n"
"fldcw __fcw\n"
"pushl %ebp\n"
"mov %esp, %ebp\n"
"jmp __start\n"
"__fcw:\n"
".short 0x037F");

void __attribute__((noreturn)) __start(int argc)
{
    __current_brk = (uintptr_t)syscall1(SYS_brk, 0);
    unsigned char *args = (unsigned char *)&argc;
    char **argv = (char **)(args + sizeof(argc));
    char **envp = (char **)(args + sizeof(argc) + sizeof(void *) * (argc + 1));
    int result = main(argc, argv, envp);
    exit(result);
}

void __stack_chk_fail_local()
{
    abort();
}

void __stack_chk_fail()
{
    abort();
}
