#include <string.h>

void *wmemset(void *dst, int val, size_t n)
{
    if(!n) return dst;
    unsigned short *buf = (unsigned short *)dst;
    while(n--)
        *buf++ = val;
    return dst;
}
