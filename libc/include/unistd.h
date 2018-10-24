#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void _exit(int status) __attribute__((noreturn));
int open(const char *pathname, int flags);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int close(int fd);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UNISTD_H
