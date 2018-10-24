[bits 32]

segment .text

global syscall0
global syscall1
global syscall2
global syscall3
global syscall4
global syscall5
global syscall6

syscall0:
    mov eax, [esp + 4]
    int 0x80
    ret

syscall1:
    push ebp
    mov ebp, esp
    push ebx
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    int 0x80
    pop ebx
    mov esp, ebp
    pop ebp
    ret

syscall2:
    push ebp
    mov ebp, esp
    push ebx
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    int 0x80
    pop ebx
    mov esp, ebp
    pop ebp
    ret

syscall3:
    push ebp
    mov ebp, esp
    push ebx
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    mov edx, [ebp + 20]
    int 0x80
    pop ebx
    mov esp, ebp
    pop ebp
    ret

syscall4:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    mov edx, [ebp + 20]
    mov esi, [ebp + 24]
    int 0x80
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

syscall5:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    mov edx, [ebp + 20]
    mov esi, [ebp + 24]
    mov edi, [ebp + 28]
    int 0x80
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

syscall6:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    mov edx, [ebp + 20]
    mov esi, [ebp + 24]
    mov edi, [ebp + 28]
    push ebp
    mov ebp, [ebp + 32]
    int 0x80
    pop ebp
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
