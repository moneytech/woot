#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <cpu.h>
#include <semaphore.h>
#include <types.h>

#include <stdio.h>

template<class T>
class MessageQueue
{
    size_t capacity;
    T *data;
    Semaphore sl, msg;
    uint tail, head;
public:
    MessageQueue(size_t capacity) :
        capacity(capacity),
        data(new T[capacity]),
        sl(capacity, "msgSlots"),
        msg(0, "msgMsgs"),
        tail(0),
        head(0)
    {
    }

    T Get(uint timeout, bool *ok)
    {
        if(!msg.Wait(timeout, false, false))
        {
            if(ok) *ok = false;
            return T();
        }
        T m = data[tail];
        tail = (tail + 1) % capacity;
        sl.Signal(nullptr);
        if(ok) *ok = true;
        return m;
    }

    T Peek(bool *ok)
    {
        bool ints = cpuDisableInterrupts();
        if(msg.GetCount())
        {
            if(ok) *ok = true;
            T m = data[tail];
            cpuRestoreInterrupts(ints);
            return m;
        }
        cpuRestoreInterrupts(ints);
        if(ok) *ok = false;
        return T();
    }

    bool Put(T m, uint timeout, bool tryput)
    {
        if(!sl.Wait(timeout, tryput, false))
            return false;
        data[head] = m;
        head = (head + 1) % capacity;
        msg.Signal(nullptr);
        return true;
    }

    ~MessageQueue()
    {
        delete[] data;
    }
};

#endif // MESSAGEQUEUE_H
