[bits 32]

segment .text
global log, logf, logl
type log function
type logf function
type logl function

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
