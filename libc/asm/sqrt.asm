[bits 32]

segment .text
global sqrt, sqrtf, sqrtl

sqrt:
    fld qword [esp + 4]
    fsqrt
    ret

sqrtf:
    fld dword [esp + 4]
    fsqrt
    ret

sqrl:
    fldt [esp + 4]
    fsqrt
    ret
