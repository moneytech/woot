#include <ctype.h>

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int tolower(int c)
{
    if(c >= 'A' && c <= 'Z')
        return c + 32;
    return c;
}

int toupper(int c)
{
    if(c >= 'a' && c <= 'z')
        return c - 32;
    return c;
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

int isxdigit(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int isalpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int isupper(int c)
{
    return (c >= 'A' && c <= 'Z');
}
