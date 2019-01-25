[bits 32]

segment .text
global cos, cosf, cosl
type cos function
type cosf function
type cosl function

cosf:
    fld dword [esp + 4]
    jmp justDoIt

cosl:
    fldt [esp + 4]
    jmp justDoIt

cos:
    fld qword [esp + 4]
justDoIt:
    fcos
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
    fcos
.done:
    ret
