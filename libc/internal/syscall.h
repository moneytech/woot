#ifndef INTERNAL_SYSCALL_H
#define INTERNAL_SYSCALL_H

// declaration of routines from asm/syscall.asm

long syscall0(long number);
long syscall1(long number, long arg1);
long syscall2(long number, long arg1, long arg2);
long syscall3(long number, long arg1, long arg2, long arg3);
long syscall4(long number, long arg1, long arg2, long arg3, long arg4);
long syscall5(long number, long arg1, long arg2, long arg3, long arg4, long arg5);
long syscall6(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

#endif // INTERNAL_SYSCALL_H
