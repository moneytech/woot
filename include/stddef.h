#ifndef STDDEF_H
#define STDDEF_H

typedef unsigned int size_t;
typedef signed int ssize_t;
typedef unsigned long long size64_t;
typedef signed long long ssize64_t;

#ifdef __GNUC__
#define offsetof(st, m) __builtin_offsetof(st, m)
#else
#define offsetof(st, m) ((size_t)&(((st *)0)->m))
#endif

#endif // STDDEF_H
