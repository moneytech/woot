#include <errno.h>
#include <signal.h>

int kill(pid_t pid, int sig)
{
    errno = ENOSYS;
    return -1;
}

__sighandler_t signal(int sig, __sighandler_t handler)
{
    return 0;
}
