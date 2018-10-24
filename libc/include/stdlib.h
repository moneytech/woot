#ifndef STDLIB_H
#define STDLIB_H

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define RAND_MAX INT_MAX

void srand(int seed);
int rand();
void exit(int status) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDLIB_H
