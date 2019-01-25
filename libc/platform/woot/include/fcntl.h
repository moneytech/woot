#ifndef FCNTL_H
#define FCNTL_H

#include <sys/stat.h>

#define O_RDONLY    00
#define O_WRONLY    01
#define O_RDWR      02
#define O_ACCMODE   03
#define O_CREAT     0100
#define O_EXCL      0200
#define O_NOCTTY    0400
#define O_TRUNC     01000
#define O_APPEND    02000
#define O_NONBLOCK  04000
#define O_DIRECTORY 0200000

int open(const char *pathname, int flags, ...);

#endif // FCNTL_H
