#include <string.h>

char *strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;
    if(!str) str = *nextp;
    str += strspn(str, delim);
    if(!*str) return NULL;
    ret = str;
    str += strcspn(str, delim);
    if(*str) *str++ = 0;
    *nextp = str;
    return ret;
}
