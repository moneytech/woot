#ifndef QUEUE_H
#define QUEUE_H

#include <types.h>

template<class T>
class Queue
{
    T *Data;
    size_t Capacity;
    uint Head;
    uint Tail;
    bool Full;
public:
    Queue(size_t capacity) :
        Data(new T[capacity]),
        Capacity(capacity)
    {
    }

    bool IsEmpty()
    {
        return (Head == Tail) && !Full;
    }

    bool IsFull()
    {
        return Full;
    }

    bool Write(T data)
    {
        if(Full) return false;
        Data[Head++] = data;
        Head %= Capacity;
        Full = Head == Tail;
        return true;
    }

    T Read(bool *ok)
    {
        if(ok) *ok = true;
        if(IsEmpty())
        {
            if(ok) *ok = false;
            return T();
        }
        T r = Data[Tail++];
        Tail %= Capacity;
        Full = false;
        return r;
    }

    T Peek()
    {
        if(IsEmpty())
            return T();
        return Data[Tail];
    }

    uint ReplaceFirst(T value, T replacement)
    {
        if(IsEmpty())
            return 0;

        for(uint i = Tail; i != Head; i = (i + 1) % Capacity)
        {
            if(Data[i] == value)
            {
                Data[i] = replacement;
                return 1;
            }
        }
        return 0;
    }

    uint ReplaceAll(T value, T replacement)
    {
        if(IsEmpty())
            return 0;

        uint replaced = 0;
        for(uint i = Tail; i != Head; i = (i + 1) % Capacity)
        {
            if(Data[i] == value)
            {
                Data[i] = replacement;
                ++replaced;
            }
        }
        return replaced;
    }

    void Clear()
    {
        for(;;)
        {
            bool ok = false;
            Read(&ok);
            if(!ok) break;
        }
    }

    ~Queue()
    {
        if(Data) delete[] Data;
    }
};

#endif // QUEUE_H
