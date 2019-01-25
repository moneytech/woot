#ifndef MMAN_H
#define MMAN_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int munmap(void *addr, size_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MMAN_H
