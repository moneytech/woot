#ifndef STDARG_H
#define STDARG_H

#define __gnuc_va_start(a, p) __builtin_va_start(a, p)
#define __gnuc_va_arg(a, t) __builtin_va_arg(a, t)
#define __gnuc_va_copy(d, s) __builtin_va_copy(d, s)
#define __gnuc_va_end(a) __builtin_va_end(a)

#define va_start(a, p) __gnuc_va_start(a, p)
#define va_arg(a, t) __gnuc_va_arg(a, t)
#define va_copy(d, s) __gnuc_va_copy(d, s)
#define va_end(a) __gnuc_va_end(a)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#endif // STDARG_H
