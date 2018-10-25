#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>

#define FILENAME_MAX 255
typedef void *FILE;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int vprintf(const char *format, va_list arg);
int printf(const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arg);
int fprintf(FILE *stream, const char *format, ...);
int vsprintf(char *str, const char *format, va_list arg);
int sprintf(char *str, const char *format, ...);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDIO_H
