#include <ctype.h>
#include <string.h>
#include <strings.h>

int bcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

void bcopy(const void *src, void *dest, size_t n)
{
    memmove(dest, src, n);
}

void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}

int ffs(int i)
{
    static const int bits = sizeof(i) * 8;
    for(int bit = 0; bit < bits; ++bit)
    {
        if((i >> bit) & 1)
            return (bit + 1);
    }
    return 0;
}

char *index(const char *s, int c)
{
    return strchr(s, c);
}

char *rindex(const char *s, int c)
{
    return strrchr(s, c);
}

int strcasecmp(const char *s1, const char *s2)
{
    for(;;)
    {
        unsigned char a = *s1++;
        unsigned char b = *s2++;
        int d = tolower(a) - tolower(b);
        if(d) return d;
        else if(!a) break;
    };
    return 0;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        unsigned char a = *s1++;
        unsigned char b = *s2++;
        int d = tolower(a) - tolower(b);
        if(d) return d;
        else if(!a) break;
    };
    return 0;
}
