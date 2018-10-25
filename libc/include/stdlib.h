#ifndef STDLIB_H
#define STDLIB_H

#include <limits.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define RAND_MAX INT_MAX

void srand(int seed);
int rand();
void exit(int status) __attribute__((noreturn));
void abort() __attribute__((noreturn));
void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void free(void *ptr);

long strtol(const char *str, char **endptr, int base);
unsigned long strtoul(const char *str, char **endptr, int base);
long long strtoll(const char *str, char **endptr, int base);
unsigned long long strtoull(const char *str, char **endptr, int base);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDLIB_H
