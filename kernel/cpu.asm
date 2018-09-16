[bits 32]

global cpuSystemHalt
cpuSystemHalt:
    mov eax, [esp + 4]
    cli
    hlt

global cpuGetEIP
cpuGetEIP:
  pop eax
  jmp eax

global cpuGetESP
cpuGetESP:
  mov eax, esp
  add eax, 4
  ret

global cpuGetEBP
cpuGetEBP:
  mov eax, ebp
  ret

global cpuLGDT
cpuLGDT:
  mov eax, [esp + 4]
  lgdt [eax]
  ret

global cpuLIDT
cpuLIDT:
  mov eax, [esp + 4]
  lidt [eax]
  ret

global cpuLTR
cpuLTR:
  mov eax, [esp + 4]
  ltr ax
  ret

global cpuFixSegments
cpuFixSegments:
  mov eax, 0x00000010 ; SEG_DATA32_KERNEL
  mov ss, ax
  mov ds, ax
  mov es, ax
  mov fs, ax
  ;mov eax, 0x0000004B ; SEG_TLS
  mov gs, ax
  jmp 0x0008:.fixCS ; SEG_CODE32_KERNEL
.fixCS:
  ret
