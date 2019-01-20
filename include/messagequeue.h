#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <cpu.h>
#include <queue.h>
#include <semaphore.h>
#include <types.h>


template<class T>
class MessageQueue
{
    Queue<T> queue;
    Semaphore msgs;
    Semaphore slts;
public:
    MessageQueue(size_t capacity) :
        queue(capacity),
        msgs(0), slts(capacity)
    {
    }

    T Get(uint timeout, bool *ok)
    {
        if(!msgs.Wait(timeout, false, false))
        {
            if(ok) *ok = false;
            return T();
        }
        T res = queue.Read(ok);
        slts.Signal(nullptr);
        return res;
    }

    T Peek(bool *ok)
    {
        bool ints = cpuDisableInterrupts();
        T res = queue.Peek(ok);
        cpuRestoreInterrupts(ints);
        return res;
    }

    uint Count()
    {
        bool ints = cpuDisableInterrupts();
        uint res = queue.Count();
        cpuRestoreInterrupts(ints);
        return res;
    }

    bool Put(T m, uint timeout, bool tryput)
    {
        if(!slts.Wait(timeout, tryput, false))
            return false;
        bool ok = queue.Write(m);
        msgs.Signal(nullptr);
        return ok;
    }

    ~MessageQueue()
    {
    }
};

#endif // MESSAGEQUEUE_H
