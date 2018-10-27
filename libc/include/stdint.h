#ifndef STDINT_H
#define STDINT_H

#include <limits.h>

#define SIZE_MAX ULONG_MAX
#define SSIZE_MAX LONG_MAX

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef unsigned long uintptr_t;
typedef signed long intptr_t;

typedef unsigned long long uintmax_t;
typedef signed long long intmax_t;

#endif // STDINT_H