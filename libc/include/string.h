#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int val, size_t n);
int memcmp(const void *ptr1, const void *ptr2, size_t n);
void *memchr(const void *ptr, int value, size_t num);

size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strdup(const char *s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
size_t strcspn(const char *s1, const char *s2);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t n);
char *strtok_r(char *str, const char *delim, char **nextp);
char *strstr(const char *haystack, const char *needle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STRING_H
