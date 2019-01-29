extern void exit(int code);
extern int main ();

/*extern void *__init_array_start;
extern void *__init_array_end;
extern void *__fini_array_start;
extern void *__fini_array_end;
typedef void (*init_fini_func)(void);*/

void __init_libc(void);

void _start()
{
    __init_libc();
    int ex = main();
    exit(ex);
}

void _init(void)
{
/*    for(void **init_func = &__init_array_start; init_func != &__init_array_end; ++init_func)
        ((init_fini_func)init_func)();*/
}

void _fini(void)
{
/*    for(void **fini_func = &__fini_array_start; fini_func != &__fini_array_end; ++fini_func)
        ((init_fini_func)fini_func)();*/
}
