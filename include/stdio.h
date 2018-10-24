#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void *FILE;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fprintf(void *f, const char *fmt, ...);
int printf(const char *fmt, ...);
int putchar(int ch);
int puts(const char *str);

#define NOT_IMPLEMENTED printf("%s not implemented yet\n", __PRETTY_FUNCTION__);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDIO_H
