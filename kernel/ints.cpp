#include <cpu.h>
#include <ints.h>
#include <irqs.h>
#include <stdio.h>
#include <thread.h>

#define VECTOR_COUNT 256

static const char *excNames[] =
{
    "Division by zero",
    "Debug exception",
    "Non-maskable interrupt",
    "Breakpoint",
    "Overflow exception",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack segment fault",
    "General protection fault",
    "Page fault",
    "Exception 15",
    "x87 FPU exception",
    "Alignment check",
    "Machine check",
    "SIMD FPU exception",
    "Virtualization exception",
    "Exception 21",
    "Exception 22",
    "Exception 23",
    "Exception 24",
    "Exception 25",
    "Exception 26",
    "Exception 27",
    "Exception 28",
    "Exception 29",
    "Security exception",
    "Exception 31"
};

Ints::Handler *Ints::Handlers[VECTOR_COUNT];

extern "C" void intsCommonHandler(Ints::State *state)
{
    Ints::CommonHandler(state);
}

void Ints::CommonHandler(Ints::State *state)
{
    int irq = state->InterruptNumber - IRQS_BASE;
    bool isIrq = irq >= 0 && irq < IRQS_COUNT;

    // handle spurious irqs
    if(isIrq)
    {
        if(IRQs::IsSpurious(irq))
        {
            IRQs::HandleSpurious(irq);
            return;
        }
    }

    bool handled = false;
    Handler *handler = Handlers[state->InterruptNumber];
    for(handled = false; !handled && handler && handler->Callback; handler = handler->Next)
        handled = handler->Callback(state, handler->Context);
    if(!handled)
    {
        // print some info about what happened
        if(isIrq)
            printf("Unhandled IRQ %d (interrupt %d)\n", irq, state->InterruptNumber);
        else
        {
            printf("Unhandled %s %d (%s)\n",
                   state->InterruptNumber < 32 ? "exception" : "interrupt",
                   state->InterruptNumber,
                   state->InterruptNumber < 32 ? excNames[state->InterruptNumber] : "hardware interrupt");

            // show what thread failed
            Thread *ct = Thread::GetCurrent();
            if(ct) printf("Thread: %d (%s)\n", ct->ID, ct->Name);
        }


        // print extra info for PF
        if(state->InterruptNumber == 14)
        {
            printf("%s when %s address %#.8x\n", state->ErrorCode & 1 ? "Page protection violation" : "Page not present",
                   state->ErrorCode & 16 ? "executing code at" : (state->ErrorCode & 2 ? "writing to" : "reading from"),
                   cpuGetCR2());
        }
        DumpState(state);
        cpuSystemHalt(state->InterruptNumber);
    }

    if(isIrq) IRQs::SendEOI(irq);
}

void Ints::RegisterHandler(uint intNo, Ints::Handler *handler)
{
    if(intNo >= VECTOR_COUNT || !handler)
        return;
    bool ints = cpuDisableInterrupts();
    handler->Next = Handlers[intNo];
    Handlers[intNo] = handler;
    cpuRestoreInterrupts(ints);
}

void Ints::UnRegisterHandler(uint intNo, Ints::Handler *handler)
{
    if(intNo >= VECTOR_COUNT || !handler)
        return;
    bool ints = cpuDisableInterrupts();
    for(Handler *prev = 0, *h = Handlers[intNo]; h; prev = h, h = h->Next)
    {
        if(handler == h)
        {
            if(prev) prev->Next = h->Next;
            else Handlers[intNo] = h->Next;
        }
    }
    cpuRestoreInterrupts(ints);
}

void Ints::DumpState(Ints::State *state)
{
    printf("EAX: %.8X EBX: %.8X ECX: %.8X EDX: %.8X\n",
           state->EAX, state->EBX, state->ECX, state->EDX);
    printf("ESI: %.8X EDI: %.8X ESP: %.8X EBP: %.8X\n",
           state->ESI, state->EDI, state->ESP, state->EBP);
    printf("CS: %.4X EIP: %.8X EFLAGS: %.8X CR2: %.8X\n",
           state->CS, state->EIP, state->EFLAGS, cpuGetCR2());
    printf("DS: %.4X ES: %.4X FS: %.4X GS: %.4X SS: %.4X\n",
           state->DS, state->ES, state->FS, state->GS, state->SS);
    if(state->CS & 0x03 || state->EFLAGS & (1 << 17))
        printf("UserSS: %.4X UserESP: %.8X\n", state->UserSS, state->UserESP);
    else printf("UserSS: N/A  UserESP: N/A\n");
}
