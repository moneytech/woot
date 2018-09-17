#ifndef TYPES_H
#define TYPES_H

typedef int intn;
typedef unsigned int uintn;
typedef unsigned int uint;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long long qword;

typedef byte uint8_t;
typedef word uint16_t;
typedef dword uint32_t;
typedef qword uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef int32_t off_t;
typedef int64_t off64_t;

typedef uint32_t uintptr_t;
typedef int32_t intptr_t;
typedef intptr_t ptrdiff_t;

typedef int64_t time_t;

#ifndef __cplusplus
typedef uint16_t wchar_t;
typedef int bool;
#define true 1
#define false 0
#define nullptr 0
#endif // __cplusplus

typedef intn pid_t;
typedef intn uid_t;
typedef intn gid_t;

#define TRUE true
#define FALSE false
#define NULL 0

#include <stddef.h>

#endif // TYPES_H
