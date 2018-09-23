#ifndef STDLIB_H
#define STDLIB_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void srand(uint seed);
uint rand();
void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void free(void *ptr);
void abort();
void *sbrk(intptr_t incr);

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(_min, _max, val) (max((_min), min((_max), (val))))
#define align(val, alignment) ((alignment) * (((val) + ((alignment) - 1)) / (alignment)))
#define swap(T, a, b) { T t = (a); (a) = (b); (b) = (t); }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDLIB_H
