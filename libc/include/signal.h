#ifndef SIGNAL_H
#define SIGNAL_H

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

void signal(int sig, void (*func)(int));

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SIGNAL_H
