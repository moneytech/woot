[bits 32]

global cpuEnableInterrupts
cpuEnableInterrupts:
    sti
    ret

global cpuDisableInterrupts
cpuDisableInterrupts:
    pushfd
    cli
    pop eax
    shr eax, 9
    and eax, 1
    ret

global cpuRestoreInterrupts
cpuRestoreInterrupts:
    mov eax, [esp + 4]
    or eax, eax
    jz .disable
    sti
    ret
.disable:
    cli
    ret

global cpuSystemHalt
cpuSystemHalt:
    mov eax, [esp + 4]
    cli
    hlt

global cpuWaitForInterrupt
cpuWaitForInterrupt:
    mov eax, [esp + 4]
    hlt

global cpuGetEIP
cpuGetEIP:
    pop eax
    jmp eax

global cpuGetESP
cpuGetESP:
    mov eax, esp
    add eax, 4
    ret

global cpuGetEBP
cpuGetEBP:
    mov eax, ebp
    ret

global cpuLGDT
cpuLGDT:
    mov eax, [esp + 4]
    lgdt [eax]
    ret

global cpuLIDT
cpuLIDT:
    mov eax, [esp + 4]
    lidt [eax]
    ret

global cpuLTR
cpuLTR:
    mov eax, [esp + 4]
    ltr ax
    ret

global cpuFixSegments
cpuFixSegments:
    mov eax, 0x00000010 ; SEG_DATA32_KERNEL
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    ;mov eax, 0x0000004B ; SEG_TLS
    mov gs, ax
    jmp 0x0008:.fixCS ; SEG_CODE32_KERNEL
.fixCS:
    ret

global _inb
_inb:
    mov edx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global _inw
_inw:
    mov edx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

global _inl
global _ind
_inl:
_ind:
    mov edx, [esp + 4]
    in eax, dx
    ret

global _outb
_outb:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, al
    ret

global _outw
_outw:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, ax
    ret

global _outl
global _outd
_outl:
_outd:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

global _insb
_insb:
    cld
    push edi
    mov edi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep insb
    pop edi
    ret

global _insw
_insw:
    cld
    push edi
    mov edi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep insw
    pop edi
    ret

global _insl
global _insd
_insl:
_insd:
    cld
    push edi
    mov edi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep insd
    pop edi
    ret

global _outsb
_outsb:
    cld
    push esi
    mov esi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep outsb
    pop esi
    ret

global _outsw
_outsw:
    cld
    push esi
    mov esi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep outsw
    pop esi
    ret

global _outsl
global _outsd
_outsl:
_outsd:
    cld
    push esi
    mov esi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep outsd
    pop esi
    ret
