#ifndef THREAD_H
#define THREAD_H

// thread routines
int thrGetCurrentID(); // get current thread id
int thrCreate(void *entry); // create new thread
int thrDelete(int id); // stop and delete (abort) specified thread
int thrSuspend(int id); // suspend specified thread
int thrResume(int id); // resume specified thread (if suspended or at interruptible sleep)
int thrSleep(int id, int ms); // make thread go to sleep for ms milliseconds (negative value means interruptible sleep)

// spinlocks
typedef volatile int SpinLock;
int thrSpinLock(SpinLock *lock);
int thrSpinRelease(SpinLock *lock);

// mutex routines
int thrMutexCreate(); // create new mutex
int thrMutexDelete(int id); // delete specified mutex
int thrMutexAcquire(int id, int timeout, int tryAcquire); // acquire specified mutex (if timeout == 0 then wait indefinitely; if tryAcquire != 0 then fail if mutex already acquired)
int thrMutexRelease(int id); // release specified mutexindefinitely
int thrMutexCancel(int id, int threadId); // fail pending thrMutexAcquire in other threads

// semaphore routines
int thrSemaphoreCreate(int initialValue); // create new semaphore
int thrSemaphoreDelete(int id); // delete specified semaphore
int thrSemaphoreWait(int id, int timeout, int tryWait); // wait for specified semaphore (if timeout == 0 then wait indefinitely; if tryWait != 0 then fail if semaphore not ready)
int thrSemaphoreSignal(int id); // signal to other threads that semaphore is ready
int thrSemaphoreReset(int id, int value); // reset count of specified semaphore to specific value
int thrSemaphoreCancel(int id, int threadId); // fail pending thrSemaphoreWait in other threads

#endif // THREAD_H
