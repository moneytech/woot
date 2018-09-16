#include <cpu.h>
#include <gdt.h>
#include <string.h>
#include <sysdefs.h>

GDTDescriptor GDT::Descriptor;
GDTEntry GDT::Entries[GDT_ENTRY_COUNT];
TSS GDT::MainTSS;

void GDT::Initialize()
{
    Descriptor.Limit = sizeof(Entries) - 1;
    Descriptor.Entries = Entries;

    for(int i = 0; i < GDT_ENTRY_COUNT; ++i)
        Entries[i].Value = 0;

    memset(&MainTSS, 0, sizeof(MainTSS));
    MainTSS.IOMapBase = 104;

    MainTSS.CS = SEG_CODE32_KERNEL;
    MainTSS.SS0 = SEG_DATA32_KERNEL;
    MainTSS.DS = SEG_DATA32_KERNEL;
    MainTSS.ES = SEG_DATA32_KERNEL;
    MainTSS.FS = SEG_DATA32_KERNEL;
    MainTSS.GS = SEG_DATA32_KERNEL;
    MainTSS.EIP = cpuGetEIP();
    MainTSS.ESP0 = cpuGetESP();

    SetEntry(SEG_CODE32_KERNEL >> 3, 0, 0xFFFFF, 0x9A, 0xC);
    SetEntry(SEG_DATA32_KERNEL >> 3, 0, 0xFFFFF, 0x92, 0xC);
    SetEntry(SEG_CODE16_KERNEL >> 3, 0, 0xFFFFF, 0x9A, 0x0);
    SetEntry(SEG_DATA16_KERNEL >> 3, 0, 0xFFFFF, 0x92, 0x0);
    SetEntry(SEG_CODE32_USER >> 3, 0, 0xFFFFF, 0xFA, 0xC);
    SetEntry(SEG_DATA32_USER >> 3, 0, 0xFFFFF, 0xF2, 0xC);
    SetEntry(SEG_MAIN_TSS >> 3, (uintptr_t)&MainTSS, sizeof(MainTSS) - 1, 0x89, 0x4);
    SetEntry(SEG_TLS >> 3, 0, 0xFFFFF, 0xF2, 0xC);

    Reload();
}

void GDT::SetEntry(uintn i, uintptr_t Base, size_t Limit, byte Access, byte Flags)
{
    Entries[i].Base23_0 = Base & 0xFFFFFF;
    Entries[i].Base31_24 = (Base >> 24) & 0xFF;
    Entries[i].Limit15_0 = Limit & 0xFFFF;
    Entries[i].Limit19_16 = (Limit >> 16) & 0x0F;
    Entries[i].Access = Access;
    Entries[i].Flags = Flags;
}

void GDT::Reload()
{
    cpuLGDT(&Descriptor);
    cpuFixSegments();
}
