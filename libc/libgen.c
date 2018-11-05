#include <libgen.h>
#include <stdlib.h>
#include <string.h>

char *basename(char *path)
{
    char *rchr = strrchr(path, '/');
    if(!rchr) return path;
    return rchr + 1;
}

char *dirname(char *path)
{
    char *rchr = strrchr(path, '/');
    if(!rchr) return ".";
    if(rchr != path) *rchr = 0;
    else return "/";
    return path;
}
