[bits 32]

global log, logf, logl

logf:
    fldln2
    fld dword [esp + 4]
    fyl2x
    ret

log:
    fldln2
    fld qword [esp + 4]
    fyl2x
    ret

logl:
    fldln2
    fldt [esp + 4]
    fyl2x
    ret
