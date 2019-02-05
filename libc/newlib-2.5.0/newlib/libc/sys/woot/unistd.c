#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

pid_t getpid()
{
    long res = syscall0(SYS_getpid);
    if(res < 0)
    {
        errno = EINVAL;
        return -1;
    }
    return res;
}

pid_t getppid(void)
{
    long res = syscall0(SYS_getppid);
    if(res < 0)
    {
        errno = EINVAL;
        return -1;
    }
    return res;
}

int setuid(uid_t uid)
{
    return seteuid(uid);
}

int setgid(gid_t gid)
{
    return setegid(gid);
}

int seteuid(uid_t euid)
{
    int res = syscall1(SYS_setuid, euid);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

int setegid(gid_t egid)
{
    int res = syscall1(SYS_setgid, egid);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

gid_t getgid(void)
{
    uid_t res = syscall0(SYS_getgid);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

uid_t geteuid(void)
{
    uid_t res = syscall0(SYS_geteuid);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

gid_t getegid(void)
{
    gid_t res = syscall0(SYS_getegid);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}
