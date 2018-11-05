#ifndef STDDEF_H
#define STDDEF_H

#include <sys/types.h>

#define NULL ((void *)0)

#ifdef __GNUC__
#define offsetof(st, m) __builtin_offsetof(st, m)
#else
#define offsetof(st, m) ((size_t)&(((st *)0)->m))
#endif

typedef long ptrdiff_t;

#endif // STDDEF_H
