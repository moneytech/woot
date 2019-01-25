#include <stdlib.h>
#include <string.h>

char *strdup(const char *s)
{
    if(!s) return NULL;
    int len = strlen(s) + 1;
    char *str = (char *)malloc(len);
    memcpy(str, s, len);
    return str;
}
