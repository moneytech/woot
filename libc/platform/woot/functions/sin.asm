[bits 32]

segment .text
global sin, sinf, sinl
type sin function
type sinf function
type sinl function

sinf:
    fld dword [esp + 4]
    jmp justDoIt

sinl:
    fldt [esp + 4]
    jmp justDoIt

sin:
    fld qword [esp + 4]
justDoIt:
    fsin
    fnstsw ax
    test ah, 0x04
    je .done
    fldpi
    fadd st0
    fxch st1
.again:
    fprem1
    fnstsw ax
    test ah, 0x04
    jne .again
    fstp st1
    fsin
.done:
    ret
