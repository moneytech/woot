#include <sys/syscall.h>
#include <woot/thread.h>

int thrGetCurrentID()
{
    return syscall0(SYS_gettid);
}

int thrCreate(void *entry)
{
    return syscall1(SYS_thread_create, (long)entry);
}

int thrDelete(int id)
{
    return syscall1(SYS_thread_delete, id);
}

int thrSuspend(int id)
{
    return syscall1(SYS_thread_suspend, id);
}

int thrResume(int id)
{
    return syscall1(SYS_thread_resume, id);
}

int thrSleep(int id, int ms)
{
    return syscall2(SYS_thread_sleep, id, ms);
}
