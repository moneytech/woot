#ifndef DIRENT_H
#define DIRENT_H

#include <limits.h>
#include <sys/types.h>

#define _DIRENT_HAVE_D_TYPE 1

#define DT_UNKNOWN 0
#define DT_REG 1
#define DT_DIR 2
#define DT_FIFO 3
#define DT_SOCK 4
#define DT_CHR 5
#define DT_BLK 6
#define DT_LNK 7

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct DIR DIR;

struct dirent
{
    ino_t d_ino;
    unsigned char d_type;
    char d_name[NAME_MAX + 1];
};

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DIRENT_H
