#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int printf(const char *fmt, ...);
int putchar(int ch);
int puts(const char *str);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDIO_H
