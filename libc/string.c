#include <string.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    unsigned char *s = (unsigned char *)src;
    while(n--) *d++ = *s++;
    return dst;
}
