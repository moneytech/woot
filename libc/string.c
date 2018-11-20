#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    unsigned char *s = (unsigned char *)src;
    while(n--) *d++ = *s++;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    unsigned char *s = (unsigned char *)src;
    if(!n || dst == src)
        return dst;
    else if(src > dst)
    {
        while(n--) *d++ = *s++;
        return dst;
    }
    d += n;
    s += n;
    while(n--) *(--d) = *(--s);
    return dst;
}

void *memset(void *dst, int val, size_t n)
{
    if(!n) return dst;
    unsigned char *buf = (unsigned char *)dst;
    while(n--)
        *buf++ = val;
    return dst;
}

int memcmp(const void *ptr1, const void *ptr2, size_t n)
{
    unsigned char *p1 = (unsigned char *)ptr1;
    unsigned char *p2 = (unsigned char *)ptr2;
    for (;n-- ;p1++, p2++)
    {
        unsigned char dif = *p1 - *p2;
        if(dif) return dif;
    }
    return 0;
}

void *memchr(const void *ptr, int value, size_t num)
{
    char *s = (char *)ptr;
    while(num--)
    {
        if(!*s++)
            return 0;
    }
    return (char *)s;
}

size_t strlen(const char *str)
{
    if(!str) return 0;
    const char *s = str;
    for (; *s; ++s);
    return s - str;
}

int strcmp(const char *s1, const char *s2)
{
    for(;;)
    {
        unsigned char a = *s1++;
        unsigned char b = *s2++;
        int d = a - b;
        if(d) return d;
        else if(!a) break;
    };
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        unsigned char a = *s1++;
        unsigned char b = *s2++;
        int d = a - b;
        if(d) return d;
        else if(!a) break;
    };
    return 0;
}

char *strdup(const char *s)
{
    if(!s) return NULL;
    int len = strlen(s) + 1;
    char *str = (char *)malloc(len);
    memcpy(str, s, len);
    return str;
}

char *strcpy(char *dst, const char *src)
{
    char *ret = dst;
    while((*dst++ = *src++));
    return ret;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    memset(dst, 0, n);
    char *ret = dst;
    for(size_t i = 0; i < n && (*dst++ = *src++););
    return ret;
}

char *strchr(const char *s, int c)
{
    while(*s != (char)c)
    {
        if(!*s++)
            return 0;
    }
    return (char *)s;
}

char *strrchr(const char *s, int c)
{
    char *ret = 0;
    do
    {
        if(*s == (char)c)
            ret = (char *)s;
    } while(*s++);
    return ret;
}

size_t strspn(const char *s1, const char *s2)
{
    size_t ret = 0;
    while(*s1 && strchr(s2, *s1++))
        ret++;
    return ret;
}

size_t strcspn(const char *s1, const char *s2)
{
    size_t ret = 0;
    while(*s1)
    {
        if(strchr(s2, *s1))
            return ret;
        else s1++, ret++;
    }
    return ret;
}

char *strcat(char *dst, const char *src)
{
    char *ret = dst;
    while(*dst) dst++;
    while((*dst++ = *src++));
    return ret;
}

char *strncat(char *dst, const char *src, size_t n)
{
    char *ret = dst;
    size_t i = 0;
    for(;i < n && (*dst); dst++);
    for(;i < n && ((*dst++ = *src++)););
    return ret;
}

char *strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;
    if(!str) str = *nextp;
    str += strspn(str, delim);
    if(!*str) return NULL;
    ret = str;
    str += strcspn(str, delim);
    if(*str) *str++ = 0;
    *nextp = str;
    return ret;
}

char *strstr(const char *haystack, const char *needle)
{
    const char *a, *b = needle;
    if(!*b) return (char *)haystack;

    for(; *haystack; ++haystack)
    {
        if(*haystack != *b)
            continue;
        a = haystack;
        for(;;)
        {
            if(!*b) return (char *)haystack;
            if(*a++ != *b++)
                break;
        }
        b = needle;
    }
    return NULL;
}
