[bits 32]

global exp, expf, expl

expf:
    fldl2e
    fmul dword [esp + 4]
    jmp justDoIt

expl:
    fldl2e
    fld qword [esp + 4]
    fmulp
    jmp justDoIt

exp:
    fldl2e
    fmul qword [esp + 4]

justDoIt:
    fst st1
    frndint
    fst st2
    fsubrp
    f2xm1
    fld1
    faddp
    fscale
    ret
