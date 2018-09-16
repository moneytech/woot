#ifndef CPU_H
#define CPU_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void cpuEnableInterrupts();
bool cpuDisableInterrupts();		// returns true if interrupts were enabled
void cpuRestoreInterrupts(bool state);	// restores interrupt state
                    // returned by cpuDisableInterrupts

void cpuSystemHalt(uintptr_t errcode); // errcode will be put into eax before halting

uintptr_t cpuGetEIP();
uintptr_t cpuGetESP();
uintptr_t cpuGetEBP();

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

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CPU_H
