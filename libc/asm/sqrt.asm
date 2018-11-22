[bits 32]

segment .text
global sqrt, sqrtf, sqrtl
type sqrt function
type sqrtf function
type sqrtl function

sqrt:
    fld qword [esp + 4]
    fsqrt
    ret

sqrtf:
    fld dword [esp + 4]
    fsqrt
    ret

sqrtl:
    fldt [esp + 4]
    fsqrt
    ret
