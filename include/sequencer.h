#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <cpu.h>

template<class T>
class Sequencer
{
    T value;
public:
    Sequencer(T initialValue) :
        value(initialValue)
    {
    }

    T GetNext()
    {
        bool state = cpuDisableInterrupts();
        T result = value++;
        cpuRestoreInterrupts(state);
        return result;
    }
};

#endif // SEQUENCER_H
