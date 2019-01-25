#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define F_OK    0
#define X_OK    1
#define W_OK    2
#define R_OK    4

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int access(const char *pathname, int mode);
int getpagesize(void);
void _exit(int status) __attribute__((noreturn));
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
off64_t lseek64(int fd, off64_t offset, int whence);
int close(int fd);
pid_t getpid(void);
int brk(void *addr);
void *sbrk(intptr_t increment);
char *getcwd(char *buf, size_t size);
char *getwd(char *buf);
char *get_current_dir_name(void);
int fsync(int fd);
int fdatasync(int fd);
int unlink(const char *pathname);
int execve(const char *filename, char *const argv[], char *const envp[]);
pid_t fork(void);
pid_t wait(int *status);
int link(const char *oldpath, const char *newpath);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UNISTD_H
