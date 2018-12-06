#ifndef DIRENT_H
#define DIRENT_H

#include <limits.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct DIR DIR;

struct dirent
{
    ino_t d_ino;
    char d_name[NAME_MAX + 1];
};

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DIRENT_H
