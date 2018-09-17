#ifndef GDT_H
#define GDT_H

#include <types.h>
#include <sysdefs.h>

#pragma pack(push, 1)

struct GDTEntry
{
    union
    {
        struct
        {
            uint64_t Limit15_0 : 16;
            uint64_t Base23_0 : 24;
            uint64_t Access : 8;
            uint64_t Limit19_16 : 4;
            uint64_t Flags : 4;
            uint64_t Base31_24 : 8;
        };
        uint64_t Value;
    };
};

struct TSS
{
    dword PreviousTSS;
    dword ESP0, SS0;
    dword ESP1, SS1;
    dword ESP2, SS2;
    dword CR3;
    dword EIP;
    dword EFLAGS;
    dword EAX, ECX, EDX, EBX;
    dword ESP, EBP, ESI, EDI;
    dword ES, CS, SS, DS, FS, GS;
    dword LDT;
    word Trap;
    word IOMapBase;
};

struct GDTDescriptor
{
    uint16_t Limit;
    GDTEntry *Entries;
};

#pragma pack(pop)

class GDT
{
    static GDTDescriptor Descriptor;
    static GDTEntry Entries[GDT_ENTRY_COUNT];
public:
    static TSS MainTSS;
    static void Initialize();
    static void SetEntry(uintn i, uintptr_t Base, size_t Limit, byte Access, byte Flags);
    static void Reload();
};

#endif // GDT_H
