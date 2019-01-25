/* int64_t _PDCLIB_seek( FILE *, int64_t, int )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

/* This is an example implementation of _PDCLIB_seek() fit for use with POSIX
   kernels.
 */

#include <stdio.h>

#ifndef REGTEST

#include "pdclib/_PDCLIB_glue.h"

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

_PDCLIB_int64_t _PDCLIB_seek( struct _PDCLIB_file_t * stream, _PDCLIB_int64_t offset, int whence )
{
    _PDCLIB_int64_t currBuffStartPos = stream->pos.offset ? ((stream->pos.offset - 1) / stream->bufsize) * stream->bufsize : 0;
    _PDCLIB_int64_t currPos = stream->bufidx + currBuffStartPos;
    _PDCLIB_int64_t newPos = 0;
    _PDCLIB_int64_t newBuffStartPos = 0;
    _PDCLIB_int64_t rc;
    struct stat st;


    switch ( whence )
    {
    case SEEK_SET:
        newPos = offset;
        break;
    case SEEK_CUR:
        newPos = currPos + offset;
        break;
    case SEEK_END:
        rc = fstat(stream->handle, &st);
        if(rc < 0) return EOF;
        newPos = st.st_size - offset;
        break;
    default:
        _PDCLIB_errno = _PDCLIB_EINVAL;
        return EOF;
        break;
    }

    if(newPos < 0) newPos = 0;
    newBuffStartPos = (newPos / stream->bufsize) * stream->bufsize;

    if(newBuffStartPos == currBuffStartPos)
    {
        stream->bufidx = newPos - newBuffStartPos;
        if(stream->bufidx > stream->bufend)
        {
            stream->bufend = stream->bufidx;
            stream->pos.offset = newBuffStartPos + stream->bufidx;
        }
        return newPos;
    }

    if(stream->status & _PDCLIB_FWRITE)
    {
        if(_PDCLIB_flushbuffer(stream) == EOF)
            return EOF;
    }

    if(lseek64(stream->handle, newBuffStartPos, SEEK_SET) != newBuffStartPos)
        return EOF;

    stream->pos.offset = newBuffStartPos;
    if(_PDCLIB_fillbuffer(stream) == EOF)
        return EOF;
    stream->bufidx = newPos % stream->bufsize;

    stream->ungetidx = 0;

    return newPos;
}

#endif

#ifdef TEST

#include "_PDCLIB_test.h"

int main( void )
{
    /* Testing covered by ftell.c */
    return TEST_RESULTS;
}

#endif
