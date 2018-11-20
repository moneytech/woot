#ifndef SETJMP_H
#define SETJMP_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*struct __jmp_buf
{
    unsigned int eax, ebx, ecx, edx;
    unsigned int esi, edi, ebp, esp;
    unsigned int eip;
};*/

#define _JBLEN 9
typedef long jmp_buf[_JBLEN];

void longjmp(jmp_buf env, int val);
int setjmp(jmp_buf env);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SETJMP_H
