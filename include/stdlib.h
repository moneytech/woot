#ifndef STDLIB_H
#define STDLIB_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void srand(uint seed);
uint rand();
void *malloc(size_t size);
void free(void *ptr);
void abort();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDLIB_H
