#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

typedef struct DIR
{
    int fd;
    struct dirent de;
} DIR;

DIR *opendir(const char *name)
{
    int fd = open(name, O_DIRECTORY);
    if(fd < 0)
    {
        errno = -fd;
        return NULL;
    }
    DIR *res = (DIR *)calloc(1, sizeof(DIR));
    if(!res)
    {
        close(fd);
        errno = ENOMEM;
        return NULL;
    }
    res->fd = fd;
    return res;
}

struct dirent *readdir(DIR *dirp)
{
    int res = syscall3(SYS_readdir, dirp->fd, (long)&dirp->de, 1);
    if(res < 0)
    {
        errno = -res;
        return NULL;
    }
    if(!dirp->de.d_name[0])
        return NULL;
    return &dirp->de;
}

int closedir(DIR *dirp)
{
    if(!dirp)
    {
        errno = EBADF;
        return -1;
    }
    int res = close(dirp->fd);
    if(res < 0)
    {
        errno = EBADF;
        return -1;
    }
    free(dirp);
}
