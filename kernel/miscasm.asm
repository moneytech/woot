[bits 32]

segment .text

global threadReturn
extern threadFinalize
extern printf
threadReturn:
  push eax
  push dword [esp + 8]
  call threadFinalize
  push .failMsg
  call printf
  cli
  hlt

.failMsg:
  db "threadReturn: threadFinalize failed", 10, 0

