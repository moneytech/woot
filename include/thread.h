#ifndef THREAD_H
#define THREAD_H

#include <ints.h>
#include <objectqueue.h>
#include <queue.h>
#include <sequencer.h>
#include <types.h>

class Mutex;
class Process;
class Semaphore;

class Thread : public ObjectQueue::Item
{
    static Sequencer<pid_t> id;
    static Thread *currentThread;
    static Thread *idleThread;
    static ObjectQueue readyThreads;
    static ObjectQueue suspendedThreads;
    static ObjectQueue sleepingThreads;
    static Thread *lastVectorStateThread;
    static Ints::Handler nmInterruptHandler;
    static bool nmInterrupt(Ints::State *state, void *context);
    static bool sleepingThreadComparer(ObjectQueue::Item *a, ObjectQueue::Item *b);

    void kernelPush(uintptr_t value);
    void freeUserStack();
public:
    enum class State
    {
        Unknown,
        Active,
        Ready,
        Suspending,
        Suspended,
        Sleeping,
        Finalized
    };

    // thread info
    pid_t ID;
    char *Name;
    ::Process *Process;
    void *EntryPoint;
    uintptr_t Argument;

    // state
    State State;

    // stacks
    size_t KernelStackSize;
    void *KernelStack;
    size_t UserStackSize;
    void *UserStack;
    size_t SignalStackSize;
    void *SignalStack;
    uintptr_t StackPointer;

    // sleeping
    int SleepTicks;
    bool InterruptibleSleep;
    bool CanChangeState;

    // floating point stuff
    void *FXSaveData;

    // signals
    uint64_t SignalMask;
    Queue<int> *SignalQueue;
    Ints::State SavedMachineState;
    int CurrentSignal;

    // finalize stuff
    int *ReturnCodePtr;
    Semaphore *Finished;
    bool DeleteFinished;

    // locking
    Mutex *WaitingMutex;
    Semaphore *WaitingSemaphore;

    static void Initialize();
    static void Finalize(Thread *thread, int returnValue);

    Thread(const char *name, class Process *process, void *entryPoint, uintptr_t argument, size_t kernelStackSize, size_t userStackSize, int *returnCodePtr, Semaphore *finished);
    static Thread *GetNext(bool doTick);
    static void Switch(Ints::State *state, Thread *thread);
    static Thread *GetCurrent();
    void Enable();
    void Yield();
    void Suspend();
    bool Resume(bool prepend);
    bool QuickResume(Ints::State *state);
    uint TicksSleep(uint ticks, bool interruptible);
    uint Sleep(uint millis, bool interruptible);
    uintptr_t AllocUserStack();
    ~Thread();
};

#endif // THREAD_H
