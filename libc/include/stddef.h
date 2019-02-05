#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void *)0)

#define offsetof(st, m) __builtin_offsetof(st, m)

#define SA_RESTART 0

typedef signed long ptrdiff_t;
typedef unsigned long size_t;
typedef int wint_t;
typedef __WCHAR_TYPE__ wchar_t;

#endif // STDDEF_H
