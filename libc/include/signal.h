#ifndef SIGNAL_H
#define SIGNAL_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define SIGABRT 1
#define SIGFPE  2
#define SIGILL  3
#define SIGINT  4
#define SIGSEGV 5
#define SIGTERM 6

#define SIG_DFL 1
#define SIG_IGN 2

typedef void (*__sighandler_t)(int);

int kill(pid_t pid, int sig);
__sighandler_t signal(int sig, __sighandler_t handler);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SIGNAL_H
