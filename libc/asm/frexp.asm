[bits 32]

global frexp, frexpf, frexpl

frexpf:
    fld dword [esp + 4]
    fld dword [esp + 8]
    jmp justDoIt

frexpl:
    fldt [esp + 4]
    fldt [esp + 12]
    jmp justDoIt

frexp:
    fld qword [esp + 4]
    mov eax, [esp + 12]
justDoIt:
    fxtract
    fxch
    fistp dword [eax]
    push dword 0x3f000000
    fmul dword [esp]
    inc eax
    pop eax
    ret
