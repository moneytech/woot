; =====================================================
; Copyright (C) 1998, 2002 by Red Hat Inc. All rights
; reserved.
;
; Permission to use, copy, modify, and distribute this
; software is freely granted, provided that this notice
; is preserved.
; =====================================================

[bits 32]

;%define __OPTIMIZE_SIZE__

global memmove
memmove:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ebx
    mov edi, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]

;  check for destructive overlap (src < dst && dst < src + length)

    cld
    cmp esi, edi
    jae  .L2
    lea ebx, [ecx + esi - 1]
    cmp edi, ebx
    ja   .L2

; IF:	 destructive overlap, must copy backwards

    add esi, ecx
    add edi, ecx
    std

%ifndef __OPTIMIZE_SIZE__

    cmp ecx, 8
    jbe .L13
.L18:

; move trailing bytes in reverse until destination address is long word aligned

    mov edx, edi
    mov ebx, ecx
    and edx, 3
    jz .L21

    mov ecx, edx
    dec esi
    dec edi
    sub ebx, ecx
    rep movsb

    mov ebx,ecx
    inc esi
    inc edi

.L21:

; move bytes in reverse, a long word at a time

    shr ecx, 2
    sub esi, 4
    sub edi, 4
    rep movsd

    add esi, 4
    add edi, 4
    mov ecx, ebx
    and ecx, 3

%endif /* !__OPTIMIZE_SIZE__ */

; handle any remaining bytes not on a long word boundary

.L13:
    dec esi
    dec edi

.L15:
    rep movsb
    jmp .L5

align 16

; ELSE:   no destructive overlap so we copy forwards

.L2:

%ifndef __OPTIMIZE_SIZE__

    cmp ecx, 8
    jbe .L3

; move any preceding bytes until destination address is long word aligned

    mov edx, edi
    mov ebx, ecx
    and edx, 3
    jz .L11
    mov ecx, 4
    sub ecx, edx
    and ecx, 3
    sub ebx, ecx
    rep movsb

    mov ecx, ebx

; move bytes a long word at a time

.L11:
    shr ecx, 2
align 4
    rep movsd

    mov ecx, ebx
    and ecx, 3

%endif /* !__OPTIMIZE_SIZE__ */

; handle any remaining bytes

.L3:
    rep movsb
.L5:
    mov eax, [ebp + 8]
    cld

    lea esp, [ebp - 12]
    pop ebx
    pop edi
    pop esi
    leave
    ret
	
