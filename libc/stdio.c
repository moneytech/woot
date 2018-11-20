#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// FIXME: freopen will fail if these are not dynamically allocated

static FILE fstdin = { STDIN_FILENO };
static FILE fstdout = { STDOUT_FILENO };
static FILE fstderr = { STDERR_FILENO };

FILE *stdin = &fstdin;
FILE *stdout = &fstdout;
FILE *stderr = &fstderr;

static int ipow(int x, int y)
{
    int v = 1;
    for(;;)
    {
        if(y & 1) v *= x;
        y >>= 1;
        if(!y) break;
        x *= x;
    }
    return v;
}

typedef int (*writeCallback)(void *arg, char c);
typedef int (*readCallback)(void *arg, char *c);

static int fwriteCallback(void *arg, char c)
{
    return fwrite(&c, sizeof(c), 1, (FILE *)arg);
}

static int freadCallback(void *arg, char *c)
{
    return fread(c, 1, 1, (FILE *)arg);
}

typedef struct stringBuffer
{
    char *buffer;
    size_t pos;
    size_t size;
} stringBuffer;

static int stringWriteCallback(void *arg, char c)
{
    stringBuffer *sb = (stringBuffer *)arg;
    if(sb->size && sb->pos < (sb->size - 1))
        sb->buffer[sb->pos++] = c;
    if(sb->pos < sb->size)
        sb->buffer[sb->pos] = 0;
    return 1;
}

static int writeString(writeCallback wc, void *wcarg, const char *str, size_t maxLen)
{
    int bw = 0;
    for(int i = 0; *str && (!maxLen || i < maxLen); ++i)
        bw += wc(wcarg, *str++);
    return bw;
}

int writeHex(writeCallback wc, void *wcarg, uint64_t value, int caps, int minDigits, int maxDigits, int measure)
{
    const static char *hexTableLo = "0123456789abcdef";
    const static char *hexTableUp = "0123456789ABCDEF";

    if(minDigits < 1)
        minDigits = 1;
    else if(minDigits > 16)
        minDigits = 16;
    if(maxDigits < 1)
        maxDigits = 1;
    else if(minDigits > 16)
        maxDigits = 16;
    if(minDigits > maxDigits)
        maxDigits = minDigits;

    char buf[20];
    int nonzero = 0;
    int i, j;
    for(i = 0, j = 0; i < 16; i++, value <<= 4)
    {
        int dig = (value >> 60) & 0x0F;
        nonzero |= dig != 0;
        char c = caps ? hexTableUp[dig] : hexTableLo[dig];
        if((16 - i <= maxDigits && nonzero) || 16 - i <= minDigits)
            buf[j++] = c;
    }
    buf[j] = 0;
    return measure ? strlen(buf) : writeString(wc, wcarg, buf, 0);
}

int writeOct(writeCallback wc, void *wcarg, uint64_t value, int minDigits, int maxDigits, int measure)
{
    const static char *octTable = "01234567";

    if(minDigits < 1)
        minDigits = 1;
    else if(minDigits > 22)
        minDigits = 22;
    if(maxDigits < 1)
        maxDigits = 1;
    else if(minDigits > 22)
        maxDigits = 22;
    if(minDigits > maxDigits)
        maxDigits = minDigits;

    char buf[24];
    int nonzero = 0;
    int i, j;
    for(i = 0, j = 0; i < 22; i++)
    {
        int dig = (value >> (63 - i * 3)) & 0x07;
        nonzero |= dig != 0;
        char c = octTable[dig];
        if((22 - i <= maxDigits && nonzero) || 22 - i <= minDigits)
            buf[j++] = c;
    }
    buf[j] = 0;
    return measure ? strlen(buf) : writeString(wc, wcarg, buf, 0);
}

int writeDec(writeCallback wc, void *wcarg, uint64_t value, int minDigits, int maxDigits, int showPlus, int measure, int unsig)
{
    const static char *decTable = "0123456789";

    if(minDigits < 1)
        minDigits = 1;
    else if(minDigits > 19)
        minDigits = 19;
    if(maxDigits < 1)
        maxDigits = 19;
    else if(maxDigits > 19)
        maxDigits = 19;
    if(minDigits > maxDigits)
        maxDigits = minDigits;

    char buf[24];
    int nonzero = 0;
    int i, j = 0;
    int64_t d = 1000000000000000000;
    if(showPlus && value >= 0)
        buf[j++] = '+';
    else if(!unsig && ((int64_t)value) < 0)
    {
        value = -value;
        buf[j++] = '-';
    }
    for(i = 0; i < 19 && d != 0; i++, d /= 10)
    {
        int dig = value / d;
        value -= dig * d;
        nonzero |= dig != 0;
        char c = decTable[dig % 10];
        if((19 - i <= maxDigits && nonzero) || 19 - i <= minDigits)
            buf[j++] = c;
    }
    buf[j] = 0;
    return measure ? strlen(buf) : writeString(wc, wcarg, buf, 0);
}

static int internalPrintf(writeCallback wc, void *wcarg, const char *fmt, va_list arg)
{
    int hashFlag = 0;
    int dot = 0;
    int width = 0;
    int precision = 0;
    int specifier = 0;
    int argWidth = 0;
    int leftJustify = 0;
    int zeroPad = 0;
    int showPlus = 0;
    int longSpec = 0;
    int halfSpec = 0;
    int maxSpec = 0;
    int sizeSpec = 0;
    int diffSpec = 0;
    int ldSpec = 0;
    char padChr = ' ';
    int bw = 0;

    while(*fmt)
    {
        char c = *fmt;
        if(!specifier)
        {
            // reset whole parser state
            hashFlag = 0;
            dot = 0;
            width = 0;
            precision = 0;
            specifier = 0;
            argWidth = 0;
            leftJustify = 0;
            zeroPad = 0;
            showPlus = 0;
            longSpec = 0;
            halfSpec = 0;
            maxSpec = 0;
            sizeSpec = 0;
            diffSpec = 0;
            ldSpec = 0;
            padChr = ' ';

            if(c == '%') specifier = 1;
            else bw += wc(wcarg, c);
        }
        else
        {
            if(c == '%')
            {
                bw += wc(wcarg, c);
                specifier = 0;
            }
            else if(c == 'h')
                ++halfSpec;
            else if(c == 'l')
                ++longSpec;
            else if(c == 'j')
                ++maxSpec;
            else if(c == 'z')
                ++sizeSpec;
            else if(c == 't')
                ++diffSpec;
            else if(c == 'L')
                ++ldSpec;
            else if(c == '.')
            {
                if(!dot) dot = 1;
                else
                {
                    bw += wc(wcarg, c);
                    specifier = 0;
                }
            }
            else if(c == '#')
            {
                if(!hashFlag) hashFlag = 1;
                else
                {
                    bw += wc(wcarg, c);
                    specifier = 0;
                }
            }
            else if(c >= '0' && c <= '9')
            {
                if(!dot)
                {
                    if(!width && c == '0' && !zeroPad)
                    {
                        padChr = '0';
                        zeroPad = 1;
                    }
                    else
                    {
                        width *= 10;
                        width += c - '0';
                    }
                }
                else
                {
                    precision *= 10;
                    precision += c - '0';
                }
            }
            else if(c == '*')
            {
                if(!dot)
                {
                    if(!argWidth && !width)
                    {
                        argWidth = 1;
                        width = va_arg(arg, int);
                    }
                    else
                    {
                        bw += wc(wcarg, c);
                        specifier = 0;
                    }
                }
                else precision = va_arg(arg, int);
            }
            else if(c == '-')
            {
                if(!width && !dot && !leftJustify)
                    leftJustify = 1;
                else
                {
                    bw += wc(wcarg, c);
                    specifier = 0;
                }
            }
            else if(c == '+')
            {
                if(!width && !dot && !showPlus)
                    showPlus = 1;
                else
                {
                    bw += wc(wcarg, c);
                    specifier = 0;
                }
            }
            else if(c == 's')
            {
                const char *str = va_arg(arg, char *);
                if(!str) str = "(null)";
                int len = strlen(str);
                if(dot) width = precision;
                int padc = width - len;
                padc = (padc < 0 || dot) ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                bw += writeString(wc, wcarg, str, width);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                specifier = 0;
            }
            else if(c == 'c')
            {
                char chr = va_arg(arg, int);
                int padc = width - 1;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                bw += wc(wcarg, chr);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                specifier = 0;
            }
            else if(c == 'p' || c == 'P' || c == 'x' || c == 'X' || c == 'o')
            {
                int pointer = 0;
                int oct = c == 'o';
                if(c == 'p' || c == 'P')
                {
                    pointer = 1;
                    dot = 1;
                    hashFlag = 1;
                    precision = sizeof(void *) * 2;
                }

                int upperCase = isupper(c);

                // apply sub-specifiers
                uintmax_t val = 0;
                if(!pointer)
                {
                    if(diffSpec)
                        val = va_arg(arg, ptrdiff_t);
                    else if(sizeSpec)
                        val = va_arg(arg, size_t);
                    else if(maxSpec)
                        val = va_arg(arg, uintmax_t);
                    else if(longSpec > 1)
                        val = va_arg(arg, unsigned long long int);
                    else if(longSpec)
                        val = va_arg(arg, unsigned long int);
                    else if(halfSpec > 1)
                        val = va_arg(arg, unsigned char);
                    else if(halfSpec)
                        val = va_arg(arg, unsigned short int);
                    else val = va_arg(arg, unsigned int);
                } else val = (uintmax_t)(uintptr_t)va_arg(arg, void *);

                int maxDigits = width ? width - (hashFlag ? 2 : 0) : 16;
                int len = oct ? writeOct(wc, wcarg, val, precision, maxDigits, 1) :
                                writeHex(wc, wcarg, val, upperCase, precision, maxDigits, 1);
                len += hashFlag ? 2 : 0;
                int padc = width - len;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                if(hashFlag)
                    bw += oct ? writeString(wc, wcarg, "0", 0) :
                                writeString(wc, wcarg, upperCase ? "0X" : "0x", 0);
                bw += oct ? writeOct(wc, wcarg, val, precision, maxDigits, 0) :
                            writeHex(wc, wcarg, val, upperCase, precision, maxDigits, 0);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                specifier = 0;
            }
            else if(c == 'd' || c == 'i' || c == 'u')
            {
                int unsig = c == 'u';
                // appy sub-specifiers
                uintmax_t val = 0;
                if(diffSpec)
                    val = va_arg(arg, ptrdiff_t);
                else if(sizeSpec)
                    val = va_arg(arg, size_t);
                else if(maxSpec)
                    val = va_arg(arg, intmax_t);
                else if(longSpec > 1)
                    val = unsig ? va_arg(arg, unsigned long long int) : va_arg(arg, long long int);
                else if(longSpec)
                    val = unsig ? va_arg(arg, unsigned long int) : va_arg(arg, long int);
                else if(halfSpec > 1)
                    val = unsig ? va_arg(arg, unsigned char) : va_arg(arg, signed char);
                else if(halfSpec)
                    val = unsig ? va_arg(arg, unsigned short int) : va_arg(arg, short int);
                else val = unsig ? va_arg(arg, unsigned int) : va_arg(arg, int);

                int maxDigits = width ? width : 19;
                int len = writeDec(wc, wcarg, val, precision, maxDigits, showPlus, 1, unsig);
                int padc = width - len;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                bw += writeDec(wc, wcarg, val, precision, maxDigits, showPlus, 0, unsig);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += wc(wcarg, padChr);
                }

                specifier = 0;
            }
            else if(c == 'f' || c == 'F')
            {
                double val = ldSpec ? va_arg(arg, long double) : va_arg(arg, double);
                int upperCase = isupper(c);
                if(isnan(val))
                    bw += writeString(wc, wcarg, upperCase ? "NAN" : "nan", 0);
                else if(isinf(val))
                    bw += writeString(wc, wcarg, val < 0 ? (upperCase ? "-INF" : "-inf") : (upperCase ? "+INF" : "+inf"), 0);
                else
                {
                    int64_t i = (int64_t)(val);
                    bw += writeDec(wc, wcarg, i, 1, -1, showPlus, 0, 0);
                    bw += wc(wcarg, '.');
                    double f = val - i;
                    if(!precision) precision = 6;
                    int64_t ai = (int64_t)(f * ipow(10, precision));
                    ai = ai < 0 ? - ai : ai;
                    bw += writeDec(wc, wcarg, ai, precision, -1, 0, 0, 0);
                }
                specifier = 0;
            }
            else
            {
                bw += wc(wcarg, c);
                specifier = 0;
            }
        }

        ++fmt;
    }
    return bw;
}

static int internalScanf(readCallback rc, void *rcarg, const char *fmt, va_list arg)
{
    printf("[stdio] internalScanf() not implemented\n");
    return 0;
}

int vprintf(const char *format, va_list arg)
{
    return vfprintf(stdout, format, arg);
}

int printf(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int res = vprintf(format, arg);
    va_end(arg);
    return res;
}

int vfprintf(FILE *stream, const char *format, va_list arg)
{
    return internalPrintf(fwriteCallback, stream, format, arg);
}

int fprintf(FILE *stream, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int res = vfprintf(stream, format, arg);
    va_end(arg);
    return res;
}

int vsprintf(char *str, const char *format, va_list arg)
{
    stringBuffer sb = { str, 0, SIZE_MAX };
    return internalPrintf(stringWriteCallback, &sb, format, arg);
}

int sprintf(char *str, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int res = vsprintf(str, format, arg);
    va_end(arg);
    return res;
}

int snprintf(char *str, size_t n, const char *format, ...)
{
    stringBuffer sb = { str, 0, n };
    va_list arg;
    va_start(arg, format);
    int res = internalPrintf(stringWriteCallback, &sb, format, arg);
    va_end(arg);
    return res;
}

int vsnprintf(char *s, size_t n, const char *format, va_list arg)
{
    stringBuffer sb = { s, 0, n };
    return internalPrintf(stringWriteCallback, &sb, format, arg);
}

int sscanf(const char *s, const char *format, ...)
{
    fprintf(stderr, "[stdio] sscanf not implemented\n");
    return EOF;
}

void clearerr(FILE *stream)
{
    if(!stream) return;
    stream->eof = 0;
    stream->error = 0;
}

void rewind(FILE *stream)
{
    if(!stream) return;
    lseek(stream->fd, 0, SEEK_SET);
    clearerr(stream);
}

int remove(const char *filename)
{
    return unlink(filename);
}

int puts(const char *str)
{
    size_t bw = fputs(str, stdout);
    if(ferror(stdout))
        return EOF;
    bw += fputs("\n", stdout);
    if(ferror(stdout))
        return EOF;
    return bw;
}

int getc(FILE *stream)
{
    return fgetc(stream);
}

FILE *tmpfile(void)
{
    // not implemented yet
    fprintf(stderr, "[stdio] tmpfile not implemented\n");
    return NULL;
}

char *strerror(int errnum)
{
    return "[stdio] strerror not implemented\n";
}

void perror(const char *s)
{
    if(s && s[0])
        fprintf(stderr, "%s: %s\n", s, strerror(errno));
    else fprintf(stderr, "%s\n", strerror(errno));
}

int fileno(FILE *stream)
{
    if(!stream)
    {
        errno = EBADF;
        return -1;
    }
    return stream->fd;
}

int putc(int character, FILE *stream)
{
    return fputc(character, stream);
}

int getchar(void)
{
    return getc(stdin);
}

int putchar(int character)
{
    return putc(character, stdout);
}

int rename(const char *oldpath, const char *newpath)
{
    errno = -ENOSYS;
    return -1;
}

FILE *fopen(const char *filename, const char *mode)
{
    // TODO: add proper '+' handling
    char *read = strchr(mode, 'r');
    char *write = strchr(mode, 'w');
    char *append = strchr(mode, 'a');
    //int rupdate = read ? read[1] == '+' : 0;
    //int wupdate = write ? write[1] == '+' : 0;
    //int aupdate = append ? append[1] == '+' : 0;
    int rw = read && write;

    FILE *f = (FILE *)calloc(1, sizeof(FILE));
    if(!f) return NULL;

    f->fd = open(filename, (rw ? O_RDWR : (read ? O_RDONLY : (write ? O_WRONLY : 0))) | (append ? O_APPEND : 0));
    if(f->fd < 0)
    {
        free(f);
        return NULL;
    }
    return f;
}

int feof(FILE *stream)
{
    if(!stream) return 1;
    return stream->eof != 0;
}

int ferror(FILE *stream)
{
    if(!stream) return 1;
    return stream->error != 0;
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream)
{
    if(!ptr || !stream)
    {
        errno = EINVAL;
        return 0;
    }
    size_t s = size * count;
    int res = read(stream->fd, ptr, s);
    if(res < 0)
    {
        stream->error = 1;
        return 0;
    }
    stream->eof = res != s;
    return res / size;
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
    if(!ptr || !stream)
    {
        errno = EINVAL;
        return 0;
    }
    size_t s = size * count;
    int res = write(stream->fd, ptr, s);
    if(res < 0)
    {
        stream->error = 1;
        return EOF;
    }
    stream->eof = res != s;
    return res / size;
}

int fseek(FILE *stream, long int offset, int whence)
{
    if(!stream)
    {
        errno = EINVAL;
        return -1;
    }
    off_t res = lseek(stream->fd, offset, whence);
    return res < 0 ? (errno = EINVAL), -1 : 0;
}

long int ftell(FILE *stream)
{
    if(!stream)
    {
        errno = EINVAL;
        return -1;
    }
    return lseek(stream->fd, 0, SEEK_CUR);
}

int fgetpos(FILE *stream, fpos_t *pos)
{
    if(!stream)
    {
        errno = EINVAL;
        return -1;
    }
    long int p = ftell(stream);
    if(p < 0) return p;
    if(pos) *pos = p;
    return 0;
}

int fsetpos(FILE *stream, const fpos_t *pos)
{
    if(!stream || !pos)
    {
        errno = EINVAL;
        return -1;
    }
    off_t res = lseek(stream->fd, *pos, SEEK_SET);
    if(res < 0)
    {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int fputc(int character, FILE *stream)
{
    unsigned char c = character;
    fwrite(&c, sizeof(c), 1, stream);
    if(ferror(stream))
        return EOF;
    return character;
}

int fputs(const char *str, FILE *stream)
{
    size_t len = strlen(str);
    size_t res = fwrite(str, sizeof(*str), len, stream);
    if(ferror(stream))
        return EOF;
    return res;
}

int fgetc(FILE *stream)
{
    unsigned char c = 0;
    fread(&c, sizeof(c), 1, stream);
    if(ferror(stream) || feof(stream))
        return EOF;
    return c;
}

char *fgets(char *str, int num, FILE *stream)
{
    int i = 0;
    for(i = 0; i < (num - 1); ++i)
    {
        int c = fgetc(stream);
        if(c == EOF)
            break;
        str[i] = c;
        if(!c || c == '\n')
        {
            ++i;
            break;
        }
    }
    if(i) str[i] = 0;
    return !i || ferror(stream) ? NULL : str;
}

int fscanf(FILE *stream, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int res = internalScanf(freadCallback, stream, format, arg);
    va_end(arg);
    return res;
}

int fflush(FILE *stream)
{
    if(!stream) return EOF;
    int res = fsync(stream->fd);
    if(res < 0)
    {
        stream->error = 1;
        return EOF;
    }
    return 0;
}

int fclose(FILE *stream)
{
    if(!stream) return EOF;
    int res = close(stream->fd);
    free(stream);
    return res < 0 ? EOF : 0;
}
