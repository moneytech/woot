#ifndef MUTEX_H
#define MUTEX_H

#include <types.h>

class Mutex
{
public:
    Mutex();
    bool Acquire(uint timeout);
    void Release();
    ~Mutex();
};

#endif // MUTEX_H
