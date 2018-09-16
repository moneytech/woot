#include <cpu.h>
#include <debugstream.h>
#include <errno.h>

#define EXTRA_RETURN

DebugStream::DebugStream(word port)
    : port(port)
{
}

int64_t DebugStream::Read(void *buffer, int64_t n)
{
    return -ENOSYS;
}

int64_t DebugStream::Write(const void *buffer, int64_t n)
{
    byte *b = (byte *)buffer;
    for(int64_t i = 0; i < n; ++i)
    {
        byte c = *b++;
#ifdef EXTRA_RETURN
        if(c == '\n')
            _outb(port, '\r');
#endif // EXTRA_RETURN
        _outb(port, c);
    }
}

DebugStream::~DebugStream()
{
}
