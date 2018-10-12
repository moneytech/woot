#include <debugstream.h>
#include <mutex.h>
#include <stdio.h>
#include <stream.h>

static DebugStream debugStream(0xE9);
static Mutex stdoutMutex;

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    stdoutMutex.Acquire(0, false);
    int res = debugStream.VWriteFmt(fmt, args);
    stdoutMutex.Release();
    va_end(args);
    return res;
}

int putchar(int ch)
{
    stdoutMutex.Acquire(0, false);
    debugStream.WriteByte(ch);
    stdoutMutex.Release();
    return ch;
}

int puts(const char *str)
{
    stdoutMutex.Acquire(0, false);
    int res = debugStream.WriteFmt("%s\n", str);
    stdoutMutex.Release();
    return res;
}
