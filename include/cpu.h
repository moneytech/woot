#ifndef CPU_H
#define CPU_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// interrupts
void cpuEnableInterrupts();
bool cpuDisableInterrupts();		// returns true if interrupts were enabled
bool cpuAreInterruptsEnabled();
void cpuRestoreInterrupts(bool state);	// restores interrupt state
                    // returned by cpuDisableInterrupts

// halt instructions
void cpuSystemHalt(uintptr_t errcode); // errcode will be put into eax before halting
void cpuWaitForInterrupt(uintptr_t debug); // as above (for debug purpose)

// control registers
dword cpuGetCR0();
void cpuSetCR0(dword value);
dword cpuGetCR2();
dword cpuGetCR3();
void cpuSetCR3(dword value);

// paging
void cpuInvalidatePage(uintptr_t addr);

// pointer registers
uintptr_t cpuGetEIP();
uintptr_t cpuGetESP();
uintptr_t cpuGetEBP();

// system table and segment registers
void cpuLGDT(void *gdt);
void cpuLIDT(void *idt);
void cpuLTR(word tr);
void cpuFixSegments();

// io port operations
byte _inb(word port);
word _inw(word port);
dword _inl(word port);
dword _ind(word port);
void _outb(word port, byte value);
void _outw(word port, word value);
void _outl(word port, dword value);
void _outd(word port, dword value);
void _insb(void *buffer, word port, size_t n);
void _insw(void *buffer, word port, size_t n);
void _insl(void *buffer, word port, size_t n);
void _insd(void *buffer, word port, size_t n);
void _outsb(const void *buffer, word port, size_t n);
void _outsw(const void *buffer, word port, size_t n);
void _outsl(const void *buffer, word port, size_t n);
void _outsd(const void *buffer, word port, size_t n);

// int n instruction
void cpuINT(byte intNo);

// floating point stuff
void cpuFXSave(void *buffer);
void cpuFXRstor(void *buffer);
void cpuInitFPU(word cw);
void cpuEnableSSE();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CPU_H
