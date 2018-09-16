#ifndef SYSDEFS_H
#define SYSDEFS_H

#define KERNEL_BASE 0xC0000000

// segments
#define SEG_CODE32_KERNEL   0x0008
#define SEG_DATA32_KERNEL   0x0010
#define SEG_CODE16_KERNEL   0x0018
#define SEG_DATA16_KERNEL   0x0020
#define SEG_CODE32_USER     0x002B
#define SEG_DATA32_USER     0x0033
#define SEG_MAIN_TSS        0x0043
#define SEG_TLS             0x4B

#define GDT_ENTRY_COUNT ((SEG_TLS + 8) / 8)

#endif // SYSDEFS_H
