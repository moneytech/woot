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
    const char *Name;

    Mutex(const char *name = nullptr);
    bool Acquire(uint timeout, bool tryAcquire = false);
    void Release();
    void Cancel(Thread *t);
    int GetCount() const;
    ~Mutex();
};

#endif // MUTEX_H
