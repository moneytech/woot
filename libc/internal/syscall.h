#ifndef INTERNAL_SYSCALL_H
#define INTERNAL_SYSCALL_H

// declaration of routines from asm/syscall.asm

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

long syscall0(long number);
long syscall1(long number, long arg1);
long syscall2(long number, long arg1, long arg2);
long syscall3(long number, long arg1, long arg2, long arg3);
long syscall4(long number, long arg1, long arg2, long arg3, long arg4);
long syscall5(long number, long arg1, long arg2, long arg3, long arg4, long arg5);
long syscall6(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // INTERNAL_SYSCALL_H
