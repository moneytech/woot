#ifndef STDLIB_H
#define STDLIB_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

uint rand();
void *malloc(size_t size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDLIB_H
