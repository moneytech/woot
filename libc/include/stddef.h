#define NULL ((void *)0)

#define offsetof(st, m) __builtin_offsetof(st, m)

typedef signed long ptrdiff_t;
typedef unsigned long size_t;
typedef int wint_t;
typedef __WCHAR_TYPE__ wchar_t;
