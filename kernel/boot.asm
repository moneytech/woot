[bits 32]

%define MULTIBOOT_MAGIC 0x1BADB002
%define MULTIBOOT_FLAGS 0x00000003

%define KERNEL_BASE        0xC0000000
%define KERNEL_PAGE_NUMBER (KERNEL_BASE >> 22)

segment .text
align 4
_multiboot_header:
  dd MULTIBOOT_MAGIC
  dd MULTIBOOT_FLAGS
  dd -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

extern kmain
extern __init_array_start
extern __init_array_end
extern __fini_array_start
extern __fini_array_end
extern __cxa_finalize
extern cpuEnableSSE
extern cpuInitFPU
global _start
_start:
    cli
    cld

    ; load page directory
    mov eax, bootPageDir - KERNEL_BASE ; yasm generates a warning here (ignore)
    mov cr3, eax

    ; enable PSE
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ; jump to higher half
    mov eax, .starthi
    jmp eax
.starthi:
    ; unmap lower half
    ; it is not needed anymore
    mov ecx, 16
    lea edi, [bootPageDir]
    xor eax, eax
    rep stosd
    mov eax, cr3 ; flush tlb
    mov cr3, eax

    lea esp, [stack.end] ; setup stack

    ; init FPU
    push 0x37F
    call cpuInitFPU
    add esp, 4

    ; enable SSE
    call cpuEnableSSE

    ; call global constructors
    push ebx    ; we need ebx register to be preserved over constructors
    mov edx, __init_array_start
    mov ecx, __init_array_end
.next_constructor:
    cmp edx, ecx
    je .constructors_done
    push ecx
    push edx
    call [edx]
    pop edx
    pop ecx
    add edx, 4
    jmp .next_constructor
.constructors_done:
    pop ebx

    add ebx, KERNEL_BASE ; fix multiboot info pointer
    push ebx
    call kmain
    add esp, 4

    ; call global destructors
    push eax    ; we want return value from kmain to be put in eax register
                ; after system halt so we save it on the stack
    push 0
    call __cxa_finalize
    add esp, 4

    mov edx, __fini_array_start
    mov ecx, __fini_array_end
.next_destructor:
    cmp edx, ecx
    je .destructors_done
    push ecx
    push edx
    call [edx]
    pop edx
    pop ecx
    add edx, 4
    jmp .next_destructor
.destructors_done:
    pop eax

    cli
    hlt

segment .data
align 0x1000
bootPageDir: ; map first 64 megs
    dd 0x00000083
    dd 0x00400083
    dd 0x00800083
    dd 0x00C00083
    dd 0x01000083
    dd 0x01400083
    dd 0x01800083
    dd 0x01C00083
    dd 0x02000083
    dd 0x02400083
    dd 0x02800083
    dd 0x02C00083
    dd 0x03000083
    dd 0x03400083
    dd 0x03800083
    dd 0x03C00083
    times (KERNEL_PAGE_NUMBER - 16) dd 0
    dd 0x00000083
    dd 0x00400083
    dd 0x00800083
    dd 0x00C00083
    dd 0x01000083
    dd 0x01400083
    dd 0x01800083
    dd 0x01C00083
    dd 0x02000083
    dd 0x02400083
    dd 0x02800083
    dd 0x02C00083
    dd 0x03000083
    dd 0x03400083
    dd 0x03800083
    dd 0x03C00083
    times (1024 - KERNEL_PAGE_NUMBER - 16) dd 0

segment .bss
stack: resb 64 << 10
.end
