[bits 32]

segment .text

global lmemset
type lmemset function
lmemset:
    push edi
    mov edi, [esp + 8]  ; ptr
    mov eax, [esp + 12] ; value
    mov ecx, [esp + 16] ; num
    or ecx, ecx ; if num == 0
    jz .done    ; then return
    rep stosd
    mov eax, [esp + 8]
.done:
    pop edi
    ret

