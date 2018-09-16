#ifndef CPU_H
#define CPU_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void cpuSystemHalt(uintptr_t errcode); // errcode will be put into eax before halting

uintptr_t cpuGetEIP();
uintptr_t cpuGetESP();
uintptr_t cpuGetEBP();

void cpuLGDT(void *gdt);
void cpuLIDT(void *idt);
void cpuLTR(word tr);
void cpuFixSegments();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CPU_H
