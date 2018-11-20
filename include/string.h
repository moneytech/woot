#ifndef STRING_H
#define STRING_H

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void *memset(void *dst, int val, size_t n);
void *wmemset(void *dst, int val, size_t n);
void *lmemset(void *dst, int val, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *bltcpy(void *dst, const void *src, size_t bpl, size_t stride, size_t lines);
void *bltmove(void *dst, const void *src, size_t bpl, size_t dstride, size_t sstride, size_t lines);
int memcmp(const void *m1, const void *m2, size_t n);
size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
int strcasecmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t wcslen(const wchar_t *str);
char *strdup(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
size_t strcspn(const char *s1, const char *s2);
char *strstr(const char *haystack, const char *needle);
char *strcat(char *dest, const char *src);
char *strtok_r(char *str, const char *delim, char **nextp);
int isdigit(int c);
int tolower(int c);
int toupper(int c);
int isspace(int c);
int isxdigit(int c);
int isalpha(int c);
int isupper(int c);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
double strtod(const char *string, char **endPtr);
char *strrand(char *buffer, size_t nChars);
char *strpbrk(const char *str, const char *separators);
char *strrpbrk(const char *str, const char *separators);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STRING_H
