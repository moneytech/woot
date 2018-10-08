#include <malloc.h>
#include <string.h>
#include <stringbuilder.h>

StringBuilder::StringBuilder(uintn maxLength) :
    MemoryStream(malloc(maxLength), maxLength + 1),
    deleteBuffer(true)
{
    memset(GetData(), 0, maxLength + 1);
}

StringBuilder::StringBuilder(char *buffer, uintn bufferSize) :
    MemoryStream(buffer, bufferSize),
    deleteBuffer(false)
{
    memset(buffer, 0, bufferSize);
}

bool StringBuilder::Clear()
{
    position = 0;
    memset(GetData(), 0, size);
    return true;
}

char *StringBuilder::GetString()
{
    return (char *)GetData();
}

void StringBuilder::ChangeCase(bool upper)
{
    char *ptr = (char *)GetData();
    for(int i = 0; i < size && *ptr; ++i)
        *ptr++ = upper ? toupper(*ptr) : tolower(*ptr);
}

size_t StringBuilder::Replace(char chr, char replacement)
{
    size_t res = 0;
    char *ptr = (char *)GetData();
    for(int i = 0; i < size && *ptr; ++i, ++ptr)
    {
        if(*ptr == chr)
        {
            *ptr = replacement;
            ++res;
        }
    }
    return res;
}

StringBuilder::~StringBuilder()
{
    if(deleteBuffer)
        free(GetData());
}
