#ifndef SYS_TYPES_H
#define SYS_TYPES_H

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef unsigned int uintptr_t;
typedef void *caddr_t;

typedef int pid_t;
typedef int uid_t;
typedef int gid_t;

typedef uint32_t ino_t;
typedef uint32_t mode_t;

typedef long off_t;
typedef long ssize_t;

typedef unsigned long clock_t;
typedef unsigned int useconds_t;
typedef int64_t sbintime_t;

#endif // SYS_TYPES_H
