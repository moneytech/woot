[bits 32]

; TODO: optimize

global memset
memset:
    push ebp
    mov ebp, esp
    push edi
    mov edi, [ebp + 8]
    mov eax, [ebp + 12]
    mov ecx, [ebp + 16]
    rep stosb
    pop edi
    mov esp, ebp
    pop ebp
    ret

global wmemset
wmemset:
    push ebp
    mov ebp, esp
    push edi
    mov edi, [ebp + 8]
    mov eax, [ebp + 12]
    mov ecx, [ebp + 16]
    rep stosw
    pop edi
    mov esp, ebp
    pop ebp
    ret

global lmemset
lmemset:
    push ebp
    mov ebp, esp
    push edi
    mov edi, [ebp + 8]
    mov eax, [ebp + 12]
    mov ecx, [ebp + 16]
    rep stosd
    pop edi
    mov esp, ebp
    pop ebp
    ret
