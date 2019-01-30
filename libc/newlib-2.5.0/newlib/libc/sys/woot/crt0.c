extern int main(int argc, char *argv[], char *envp[]);
extern void __init_libc(void);
extern void _exit(int);
extern int printf(const char *, ...);

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
    printf("%: %s not implemented\n", __FILE__, __FUNCTION__);
}

void _fini(void)
{
    printf("%: %s not implemented\n", __FILE__, __FUNCTION__);
}
