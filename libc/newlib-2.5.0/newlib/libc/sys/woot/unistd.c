#include <sys/syscall.h>
#include <unistd.h>

uid_t geteuid(void)
{
    return syscall0(SYS_geteuid);
}

gid_t getegid(void)
{
    return syscall0(SYS_getegid);
}
