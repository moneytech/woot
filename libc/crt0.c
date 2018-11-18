#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

extern int main(int argc, char *argv[], char *envp[]);
extern void __init_libc();

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
    __init_libc();
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
