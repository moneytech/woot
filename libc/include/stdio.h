#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>

#define EOF (-1)
#define FILENAME_MAX 255
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct FILE
{
    int fd;
    int eof;
    int error;
} FILE;

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

void clearerr(FILE *stream);
void rewind(FILE *stream);

FILE *fopen(const char *filename, const char *mode);
int feof(FILE *stream);
int ferror(FILE *stream);
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
int fclose(FILE *stream);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDIO_H
