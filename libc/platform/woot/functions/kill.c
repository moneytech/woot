#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

int kill(pid_t pid, int sig)
{
    printf("kill not implemented\n");
    errno = ENOSYS;
    return -1;
}
