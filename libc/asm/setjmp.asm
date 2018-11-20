[bits 32]

global setjmp
setjmp:
    push ebp
    mov ebp, esp
    push edi

    mov edi, [ebp + 8]  ; jmp_buf

    mov [edi], eax
    mov [edi + 4], ebx
    mov [edi + 8], ecx
    mov [edi + 12], edx
    mov [edi + 16], esi

    mov eax, [ebp - 4]  ; original edi
    mov [edi + 20], eax

    mov eax, [ebp]      ; original ebp
    mov [edi + 24], eax

    mov eax, esp        ; calculate original esp
    add eax, 12
    mov [edi + 28], eax

    mov eax, [ebp + 4]  ; original eip
    mov [edi + 32], eax

    pop edi
    xor eax, eax
    mov esp, ebp
    pop ebp
    ret

global longjmp
longjmp:
    push ebp
    mov ebp, esp

    mov edi, [ebp + 8]  ; jmp_buf
    mov eax, [ebp + 12] ; val
    mov [edi], eax

    mov ebp, [edi + 24]

    cli

    mov esp, [edi + 28]
    push dword [edi + 32]

    mov eax, [edi]
    mov ebx, [edi + 4]
    mov ecx, [edi + 8]
    mov edx, [edi + 12]
    mov esi, [edi + 16]
    mov edi, [edi + 20]

    sti

    ret
