[bits 32]

segment .text
global fmod, fmodf, fmodl
type fmod function
type fmodf function
type fmodl function

fmodf:
    fld dword [esp + 8]
    fld dword [esp + 4]
    jmp justDoIt

fmodl:
    fldt [esp + 16]
    fldt [esp + 4]
    jmp justDoIt

fmod:
    fld qword [esp + 12]
    fld qword [esp + 4]

justDoIt:
    fprem
    fstsw ax
    sahf
    jp justDoIt
    fstp st1
    ret
