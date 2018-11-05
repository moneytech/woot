#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void _exit(int status) __attribute__((noreturn));
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int close(int fd);
pid_t getpid();
int brk(void *addr);
void *sbrk(intptr_t increment);
char *getcwd(char *buf, size_t size);
char *getwd(char *buf);
char *get_current_dir_name(void);
int fsync(int fd);
int fdatasync(int fd);
int unlink(const char *pathname);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UNISTD_H
