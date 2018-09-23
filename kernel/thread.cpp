#include <cpu.h>
#include <gdt.h>
#include <malloc.h>
#include <miscasm.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdefs.h>
#include <thread.h>
#include <time.h>

extern "C" void *kmain;

extern "C" void threadFinalize(Thread *thread, int returnValue)
{
    printf("[thread] threadFinalize not implemented");
    cpuSystemHalt(0x654321AA);
}

static void idleThreadProc()
{
    for(;;) cpuWaitForInterrupt(0x1D1E1D1E);
}

Sequencer<pid_t> Thread::id(1);
Thread *Thread::currentThread = nullptr;
Thread *Thread::idleThread = nullptr;
ObjectQueue Thread::readyThreads;
ObjectQueue Thread::suspendedThreads;
ObjectQueue Thread::sleepingThreads;
Thread *Thread::lastVectorStateThread = nullptr;
Ints::Handler Thread::nmInterruptHandler = { nullptr, Thread::nmInterrupt, nullptr };

bool Thread::nmInterrupt(Ints::State *state, void *context)
{
    cpuSetCR0(cpuGetCR0() & ~0x08);
    if(lastVectorStateThread && lastVectorStateThread->FXSaveData)
        cpuFXSave(lastVectorStateThread->FXSaveData);
    Thread *ct = currentThread;
    if(!ct || !ct->FXSaveData)
        return false;
    cpuFXRstor(ct->FXSaveData);
    lastVectorStateThread = ct;
    return true;
}

bool Thread::sleepingThreadComparer(Item *a, Item *b)
{
    return (a == b) && ((Thread *)a)->InterruptibleSleep;
}

void Thread::kernelPush(uintptr_t value)
{
    *(uintptr_t *)(StackPointer -= 4) = value;
}

void Thread::Initialize()
{
    bool ints = cpuDisableInterrupts();

    //if(!kernelProcess)
    //    cpuSystemHalt(); // processInitialize() has not been called

    Thread *mainThread = new Thread("main kernel thread", nullptr, kmain, 0, ~0, 0, nullptr, nullptr);
    currentThread = mainThread;
    //processAddThread(kernelProcess, mainThread);

    idleThread = new Thread("idle thread", nullptr, (void *)idleThreadProc, 0, 0, 0, nullptr, nullptr);
    //processAddThread(kernelProcess, idleThread);
    idleThread->State = State::Ready;

    //GDT::MainTSS.CR3 = kernelProcess->AddressSpace;
    lastVectorStateThread = currentThread;
    Ints::RegisterHandler(7, &nmInterruptHandler);

    cpuRestoreInterrupts(ints);
}

Thread::Thread(const char *name, class Process *process, void *entryPoint, uintptr_t argument, size_t kernelStackSize, size_t userStackSize, int *returnCodePtr, Semaphore *finished) :
    ID(id.GetNext()),
    Name(strdup(name)),
    Process(process),
    EntryPoint(entryPoint),
    Argument(argument),
    State(State::Unknown),
    KernelStackSize(kernelStackSize ? kernelStackSize : DEFAULT_STACK_SIZE),
    KernelStack(kernelStackSize == ~0 ? nullptr : calloc(1, KernelStackSize)),
    UserStackSize(userStackSize ? userStackSize : DEFAULT_STACK_SIZE),
    UserStack(nullptr),
    SignalStackSize(DEFAULT_SIGNAL_STACK_SIZE),
    SignalStack(nullptr),
    StackPointer(KernelStackSize + (uintptr_t)KernelStack),
    SleepTicks(0),
    InterruptibleSleep(false),
    CanChangeState(false),
    FXSaveData(memalign(16, 512)),
    SignalMask(~0),
    SignalQueue(nullptr),
    CurrentSignal(-1),
    ReturnCodePtr(returnCodePtr),
    Finished(finished)
{
    cpuFXSave(FXSaveData);  // FIXME: should be initialized to known good state

    // initialize stack
    kernelPush(argument);                // argument
    kernelPush((uintptr_t)threadReturn); // return address

    uintptr_t initStackPointer = StackPointer;

    // this stack layout here MUST match interrupt stack defined in ints.h
    kernelPush(0x00000202);            // EFLAGS
    kernelPush(SEG_CODE32_KERNEL);     // CS
    kernelPush((uintptr_t)entryPoint); // EIP

    kernelPush(0);                    // error code
    kernelPush(0);                    // interrupt number

    kernelPush(0);                    // EAX
    kernelPush(0);                    // ECX
    kernelPush(0);                    // EDX
    kernelPush(0);                    // EBX
    kernelPush(0);                    // EBP
    kernelPush(0);                    // ESI
    kernelPush(0);                    // EDI

    kernelPush(SEG_DATA32_KERNEL);    // DS
    kernelPush(SEG_DATA32_KERNEL);    // ES
    kernelPush(SEG_DATA32_KERNEL);    // FS
    kernelPush(SEG_DATA32_KERNEL);    // GS
    kernelPush(SEG_DATA32_KERNEL);    // SS

    kernelPush(0);                    // ESP - ignored

    StackPointer = initStackPointer;
}

Thread *Thread::GetNext(bool doTick)
{
    // handle sleeping threads
    if(doTick)
    {
        sleepingThreads.ForEach([](Item *it) -> bool
        {
            Thread *t = (Thread *)it;
            if(t->SleepTicks > 0)
                --t->SleepTicks;
            return false;
        });
    }
    sleepingThreads.ForEach([](Item *it) -> bool
    {
        Thread *t = (Thread *)it;
        if(!t->SleepTicks)
        {
            sleepingThreads.Remove(t, nullptr);
            t->State = State::Ready;
            t->InterruptibleSleep = false;
            if(t->CanChangeState)
            {
                uint32_t *stack = (uint32_t *)t->StackPointer;
                stack[0] = t->SleepTicks;
            }
            t->CanChangeState = false;
            readyThreads.Add(t, false);
            return true;
        }
        return false;
    });

    // get next thread from queue
    if(currentThread && currentThread != idleThread &&
            currentThread->State != State::Sleeping)
    {
        if(currentThread->State != State::Suspending)
        {
            currentThread->State = State::Ready;
            readyThreads.Add(currentThread, false);
        }
        else
        {
            currentThread->State = State::Suspended;
            suspendedThreads.Add(currentThread, false);
        }
    }
    Thread *t = (Thread *)readyThreads.Get();
    t = t ? t : idleThread;
    return t;
}

void Thread::Switch(Ints::State *state, Thread *thread)
{
    if(currentThread == thread)
        return; // nothing to be done here

    if(currentThread)
        currentThread->StackPointer = state->ESP;
    currentThread->State = State::Ready;

    GDT::MainTSS.ESP0 = (uintptr_t)thread->KernelStack +
            thread->KernelStackSize; // initial thread->StackPointer;
    state->ESP = thread->StackPointer;

    state->GS = SEG_TLS; // make sure GS points to SEG_TLS
    //gdtSetEntry(SEG_TLS >> 3, (uintptr_t)thread->PThread, 0xFFFFF, 0xF2, 0xC); // make SEG_TLS point to PThread structure
    GDT::Reload();

    cpuSetCR0(cpuGetCR0() | 0x08); // set TS bit
    /*if(thread->Process)
    {
        uintptr_t _cr3 = cpuGetCR3();
        uintptr_t newCr3 = thread->Process->AddressSpace;
        gdtMainTSS.CR3 = newCr3;
        if(_cr3 != newCr3) // avoid unnecesary tlb flushing
            cpuSetCR3(newCr3);
    }*/

    currentThread = thread;
    currentThread->State = State::Active;
}

Thread *Thread::GetCurrent()
{
    bool ints = cpuDisableInterrupts();
    Thread *res = currentThread;
    cpuRestoreInterrupts(ints);
    return res;
}

void Thread::Enable()
{
    bool ints = cpuDisableInterrupts();
    suspendedThreads.Add(this, false);
    this->State = State::Suspended;
    cpuRestoreInterrupts(ints);
}

void Thread::Yield()
{
    bool ints = cpuDisableInterrupts();
    Time::FakeTick();
    cpuRestoreInterrupts(ints);
}

void Thread::Suspend()
{
    bool ints = cpuDisableInterrupts();
    if(this == currentThread)
    {
        currentThread->State = State::Suspending;
        Time::FakeTick();
        cpuRestoreInterrupts(ints);
        return;
    }
    if(!readyThreads.Remove(this, nullptr))
    {
        cpuRestoreInterrupts(ints);
        return;
    }
    State = State::Suspended;
    suspendedThreads.Add(this, false);
    cpuRestoreInterrupts(ints);
}

bool Thread::Resume(bool prepend)
{
    bool ints = cpuDisableInterrupts();
    if(suspendedThreads.Remove(this, nullptr))
    {
        State = State::Ready;
        readyThreads.Add(this, prepend);
        cpuRestoreInterrupts(ints);
        return true;
    }
    if(sleepingThreads.Remove(this, sleepingThreadComparer))
    {
        if(CanChangeState)
        {
            uintptr_t *stack = (uintptr_t *)StackPointer;
            stack[0] = SleepTicks;
        }

        InterruptibleSleep = false;
        CanChangeState = false;
        SleepTicks = 0;
        State = State::Ready;
        readyThreads.Add(this, prepend);
        cpuRestoreInterrupts(ints);
        return true;
    }
    cpuRestoreInterrupts(ints);
    return false;
}

bool Thread::QuickResume(Ints::State *state)
{
    bool ints = cpuDisableInterrupts();
    if(!Resume(true)) return false;
    Switch(state, GetNext(true));
    cpuRestoreInterrupts(ints);
    return true;
}

uint Thread::TicksSleep(uint ticks, bool interruptible)
{
    bool ints = cpuDisableInterrupts();
    readyThreads.Remove(this, nullptr);
    suspendedThreads.Remove(this, nullptr);
    sleepingThreads.Remove(this, nullptr);
    State = State::Sleeping;
    SleepTicks = ticks;
    InterruptibleSleep = interruptible;
    sleepingThreads.Add(this, false);
    uint result = 0;
    if(currentThread == this)
    {
        Time::isFakeTick = true;
        CanChangeState = true;
        // TODO: Get rid of that asm
        asm("push $0x1234abcd\n"
            "int $0x28\n"
            "pop %%eax": "=a"(result));
    }
    cpuRestoreInterrupts(ints);
    return result;
}

uint Thread::Sleep(uint millis, bool interruptible)
{
    uint64_t tickFreq = Time::GetTickFrequency();
    uint ticks = ((tickFreq * millis) / 1000) + 1; // +1 make sure it's at
                                                   // least as long as
                                                   // specified (may be longer)
    uint64_t nanosPerTick = max(1, 1000000000 / tickFreq);
    uint ticksLeft = TicksSleep(ticks, interruptible);
    return (ticksLeft * nanosPerTick) / 1000000;
}

Thread::~Thread()
{
    if(Name) free(Name);
    if(KernelStack) free(KernelStack);
    if(UserStack) free(UserStack);
    if(SignalStack) free(SignalStack);
    if(FXSaveData) free(FXSaveData);
    if(SignalQueue) delete SignalQueue;
    if(Finished) delete Finished;
}