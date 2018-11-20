#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define EOF (-1)
#define FILENAME_MAX 255
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef long long fpos_t;

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
int snprintf(char *str, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

int sscanf(const char *s, const char *format, ...);

void clearerr(FILE *stream);
void rewind(FILE *stream);
int remove(const char *filename);
int puts(const char *str);
int getc(FILE *stream);
FILE *tmpfile(void);
char *strerror(int errnum);
void perror(const char *s);
int fileno(FILE *stream);
int putc(int character, FILE *stream);
int getchar(void);
int putchar(int character);
int rename(const char *oldpath, const char *newpath);

FILE *fopen(const char *filename, const char *mode);
int feof(FILE *stream);
int ferror(FILE *stream);
size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
int fseek(FILE *stream, long int offset, int whence);
long int ftell(FILE *stream);
int fgetpos(FILE *stream, fpos_t *pos);
int fsetpos(FILE *stream, const fpos_t *pos);
int fputc(int character, FILE *stream);
int fputs(const char *str, FILE *stream);
int fgetc(FILE *stream);
char *fgets(char *str, int num, FILE *stream);
int fscanf(FILE *stream, const char *format, ...);
int fflush(FILE *stream);
int fclose(FILE *stream);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // STDIO_H
