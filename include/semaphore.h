#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <ints.h>
#include <queue.h>
#include <types.h>

class Thread;

// FIXME: deleted thread can cause problems if it's still on some waiter list

class Semaphore
{
    volatile int Count;
    Queue<Thread *> *Waiters;
public:
    Semaphore(int count);
    bool Wait(uint timeout, bool tryWait, bool disableInts);
    void Signal(Ints::State *state); // passing state != 0 makes this method usable in ISRs
    void Cancel(Thread *t);
    ~Semaphore();
};

#endif // SEMAPHORE_H
