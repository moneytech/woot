#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

extern int main(int argc, char *argv[], char *envp[]);
extern void __init_libc();

void __attribute__((noreturn)) _start(int argc)
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
