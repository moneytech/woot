#ifndef MUTEX_H
#define MUTEX_H

#include <queue.h>
#include <types.h>

class Thread;

class Mutex
{
    volatile int Count;
    volatile Thread *Owner;
    Queue<Thread *> *Waiters;
public:
    static Mutex GlobalLock;

    Mutex();
    bool Acquire(uint timeout, bool tryAcquire = false);
    void Release();
    void Cancel(Thread *t);
    ~Mutex();
};

#endif // MUTEX_H
