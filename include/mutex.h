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
    Mutex();
    bool Acquire(uint timeout, bool tryAcquire = false);
    void Release();
    ~Mutex();
};

#endif // MUTEX_H
