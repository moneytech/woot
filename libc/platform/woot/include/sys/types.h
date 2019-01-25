#ifndef SYS_TYPES_H
#define SYS_TYPES_H

#include <pdclib/_PDCLIB_config.h>

typedef _PDCLIB_size size_t;
typedef signed int ssize_t;

typedef unsigned short nlink_t;
typedef unsigned long ino_t;

typedef unsigned long blksize_t;
typedef unsigned long blkcnt_t;

typedef unsigned long dev_t;
typedef unsigned short uid_t;
typedef unsigned short gid_t;

typedef long pid_t;
#define __pid_t_defined

typedef long off_t;
typedef long long off64_t;

typedef unsigned short mode_t;
#define __mode_t_defined

#endif // SYS_TYPES_H
