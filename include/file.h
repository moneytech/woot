#ifndef FILE_H
#define FILE_H

#include <types.h>

class DEntry;

class File
{
public:
    ::DEntry *DEntry;
    File();
    virtual ~File();
};

#endif // FILE_H
