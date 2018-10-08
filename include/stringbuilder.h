#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

#include <types.h>
#include <memorystream.h>

class StringBuilder : public MemoryStream
{
    bool deleteBuffer = false;
public:
    StringBuilder(uintn maxLength);
    StringBuilder(char *buffer, uintn bufferSize);
    bool Clear();
    char *GetString();
    void ChangeCase(bool upper);
    size_t Replace(char chr, char replacement);
    virtual ~StringBuilder();
};

#endif // STRINGBUILDER_H
