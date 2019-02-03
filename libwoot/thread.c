#include <sys/syscall.h>
#include <woot/thread.h>

int thrGetCurrentID()
{
    return syscall0(SYS_gettid);
}

int thrCreate(void *entry, int finishedSemaphore, int *retVal, void *arg)
{
    return syscall4(SYS_thread_create, (long)entry, finishedSemaphore, (long)retVal, (long)arg);
}

int thrDelete(int id, int retVal)
{
    return syscall2(SYS_thread_delete, id, retVal);
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

int thrSpinLock(SpinLock *lock)
{
    while(__sync_lock_test_and_set(lock, 1)) // gcc extension
        while(*lock);
}

int thrSpinRelease(SpinLock *lock)
{
    *lock = 0;
}

int thrMutexCreate()
{
    return syscall0(SYS_mutex_create);
}

int thrMutexDelete(int id)
{
    return syscall1(SYS_mutex_delete, id);
}

int thrMutexAcquire(int id, int timeout, int tryAcquire)
{
    return syscall3(SYS_mutex_acquire, id, timeout, tryAcquire);
}

int thrMutexRelease(int id)
{
    return syscall1(SYS_mutex_release, id);
}

int thrMutexCancel(int id, int threadId)
{
    return syscall2(SYS_mutex_cancel, id, threadId);
}

int thrSemaphoreCreate(int initialValue)
{
    return syscall1(SYS_semaphore_create, initialValue);
}

int thrSemaphoreDelete(int id)
{
    return syscall1(SYS_semaphore_delete, id);
}

int thrSemaphoreWait(int id, int timeout, int tryWait)
{
    return syscall3(SYS_semaphore_wait, id, timeout, tryWait);
}

int thrSemaphoreSignal(int id)
{
    return syscall1(SYS_semaphore_signal, id);
}

int thrSemaphoreReset(int id, int value)
{
    return syscall2(SYS_semaphore_reset, id, value);
}

int thrSemaphoreCancel(int id, int threadId)
{
    return syscall2(SYS_semaphore_cancel, id, threadId);
}
