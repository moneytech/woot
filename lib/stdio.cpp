#include <stdio.h>
#include <stream.h>

extern "C" Stream *debugStream;

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = debugStream->VWriteFmt(fmt, args);
    va_end(args);
    return res;
}

int putchar(int ch)
{
    debugStream->WriteByte(ch);
    return ch;
}

int puts(const char *str)
{
    return debugStream->WriteFmt("%s\n", str);
}
