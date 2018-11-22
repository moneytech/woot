[bits 32]

segment .text

global syscall0, syscall1, syscall2, syscall3, syscall4, syscall5, syscall6
type syscall0 function
type syscall1 function
type syscall2 function
type syscall3 function
type syscall4 function
type syscall5 function
type syscall6 function

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
