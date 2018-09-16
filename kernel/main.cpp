#include <gdt.h>

extern "C" int kmain(void *mbootInfo)
{
    GDT::Initialize();

    unsigned short *video = (unsigned short *)0xC00B8000;
    video[0] = 0x1F00 | 'X';
    return 0xD007D007;
}
