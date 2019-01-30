extern int main(int argc, char *argv[], char *envp[]);
extern void __init_libc(void);
extern void _exit(int);

extern char **environ;

void __attribute__((noreturn)) _start(int argc)
{
    __init_libc();
    unsigned char *args = (unsigned char *)&argc;
    char **argv = (char **)(args + sizeof(argc));
    char **envp = (char **)(args + sizeof(argc) + sizeof(void *) * (argc + 1));
    environ = envp;
    int result = main(argc, argv, envp);
    for(;;) _exit(result);
}

void _init(void)
{
    asm("cli\nhlt\n");
//    for(void **init_func = &__init_array_start; init_func != &__init_array_end; ++init_func)
//        ((init_fini_func)init_func)();
}

void _fini(void)
{
    asm("cli\nhlt\n");
//    for(void **fini_func = &__fini_array_start; fini_func != &__fini_array_end; ++fini_func)
//        ((init_fini_func)fini_func)();
}
