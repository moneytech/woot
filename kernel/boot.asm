[bits 32]

%define MULTIBOOT_MAGIC 0x1BADB002
%define MULTIBOOT_FLAGS 0x00000003

segment .text
align 4
_multiboot_header:
  dd MULTIBOOT_MAGIC
  dd MULTIBOOT_FLAGS
  dd -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

extern kmain
global _start
_start:
    cli
    cld
    lea esp, [stack.end]
    push ebx
    call kmain
    add esp, 4
    cli
    hlt

segment .bss
stack: resb 64 << 10
.end
