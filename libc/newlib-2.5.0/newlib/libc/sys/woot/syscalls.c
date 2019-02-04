#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*char *_environ[] =
{
    "NOT_IMPLEMENTED=1",
    NULL
};

char **environ = _environ;*/

extern void *end;
uintptr_t __current_brk = (uintptr_t)&end;

long syscall0(long number);
long syscall1(long number, long arg1);
long syscall2(long number, long arg1, long arg2);
long syscall3(long number, long arg1, long arg2, long arg3);
long syscall4(long number, long arg1, long arg2, long arg3, long arg4);
long syscall5(long number, long arg1, long arg2, long arg3, long arg4, long arg5);
long syscall6(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

asm(
".intel_syntax noprefix\n"
".global syscall0\n"
".type syscall0, @function\n"
"syscall0:\n"
"mov eax, [esp + 4]\n"
"int 0x80\n"
"ret\n"
".att_syntax\n"
);

asm(
".intel_syntax noprefix\n"
".type syscall1, @function\n"
".global syscall1\n"
"syscall1:\n"
"push ebp\n"
"mov ebp, esp\n"
"push ebx\n"
"mov eax, [ebp + 8]\n"
"mov ebx, [ebp + 12]\n"
"int 0x80\n"
"pop ebx\n"
"mov esp, ebp\n"
"pop ebp\n"
"ret\n"
".att_syntax\n"
);

asm(
".intel_syntax noprefix\n"
".type syscall2, @function\n"
".global syscall2\n"
"syscall2:\n"
"push ebp\n"
"mov ebp, esp\n"
"push ebx\n"
"mov eax, [ebp + 8]\n"
"mov ebx, [ebp + 12]\n"
"mov ecx, [ebp + 16]\n"
"int 0x80\n"
"pop ebx\n"
"mov esp, ebp\n"
"pop ebp\n"
"ret\n"
".att_syntax\n"
);

asm(
".intel_syntax noprefix\n"
".type syscall3, @function\n"
".global syscall3\n"
"syscall3:\n"
"push ebp\n"
"mov ebp, esp\n"
"push ebx\n"
"mov eax, [ebp + 8]\n"
"mov ebx, [ebp + 12]\n"
"mov ecx, [ebp + 16]\n"
"mov edx, [ebp + 20]\n"
"int 0x80\n"
"pop ebx\n"
"mov esp, ebp\n"
"pop ebp\n"
"ret\n"
".att_syntax\n"
);

asm(
".intel_syntax noprefix\n"
".type syscall4, @function\n"
".global syscall4\n"
"syscall4:\n"
"push ebp\n"
"mov ebp, esp\n"
"push ebx\n"
"push esi\n"
"mov eax, [ebp + 8]\n"
"mov ebx, [ebp + 12]\n"
"mov ecx, [ebp + 16]\n"
"mov edx, [ebp + 20]\n"
"mov esi, [ebp + 24]\n"
"int 0x80\n"
"pop esi\n"
"pop ebx\n"
"mov esp, ebp\n"
"pop ebp\n"
"ret\n"
".att_syntax\n"
);

asm(
".intel_syntax noprefix\n"
".type syscall5, @function\n"
".global syscall5\n"
"syscall5:\n"
"push ebp\n"
"mov ebp, esp\n"
"push ebx\n"
"push esi\n"
"push edi\n"
"mov eax, [ebp + 8]\n"
"mov ebx, [ebp + 12]\n"
"mov ecx, [ebp + 16]\n"
"mov edx, [ebp + 20]\n"
"mov esi, [ebp + 24]\n"
"mov edi, [ebp + 28]\n"
"int 0x80\n"
"pop edi\n"
"pop esi\n"
"pop ebx\n"
"mov esp, ebp\n"
"pop ebp\n"
"ret\n"
".att_syntax\n"
);

asm(
".intel_syntax noprefix\n"
".type syscall6, @function\n"
".global syscall6\n"
"syscall6:\n"
"push ebp\n"
"mov ebp, esp\n"
"push ebx\n"
"push esi\n"
"push edi\n"
"mov eax, [ebp + 8]\n"
"mov ebx, [ebp + 12]\n"
"mov ecx, [ebp + 16]\n"
"mov edx, [ebp + 20]\n"
"mov esi, [ebp + 24]\n"
"mov edi, [ebp + 28]\n"
"push ebp\n"
"mov ebp, [ebp + 32]\n"
"int 0x80\n"
"pop ebp\n"
"pop edi\n"
"pop esi\n"
"pop ebx\n"
"mov esp, ebp\n"
"pop ebp\n"
"ret\n"
".att_syntax\n"
);



void __init_libc(void)
{
    __current_brk = (uintptr_t)syscall1(SYS_brk, 0);
}

int *_get_errno_ptr()
{
    static int __errno = 0;
    return &__errno;
}

int getpagesize(void)
{
    return 4096;
}

void _exit(int status)
{
    for(;;) syscall1(SYS_exit, status);
}

int close(int fd)
{
    int res = syscall1(SYS_close, fd);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

int execve(char *name, char **argv, char **env)
{
    fprintf(stderr, "execve not implemented\n");
    errno = ENOSYS;
    return -1;
}

int fork()
{
    fprintf(stderr, "fork not implemented\n");
    errno = ENOSYS;
    return -1;
}

int fstat(int fd, struct stat *buf)
{
    long res = syscall2(SYS_fstat, fd, (long)buf);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

int fstat64(int fd, struct stat64 *buf)
{
    return fstat(fd, (struct stat *)buf);
}

int getpid()
{
    long res = syscall0(SYS_getpid);
    if(res < 0)
    {
        errno = EINVAL;
        return -1;
    }
    return res;
}

int isatty(int fd)
{
    return fd < 3;
}

int kill(int pid, int sig)
{
    fprintf(stderr, "kill not implemented\n");
    errno = ENOSYS;
    return -1;
}

int link(char *old, char *new)
{
    fprintf(stderr, "link not implemented\n");
    errno = ENOSYS;
    return -1;
}

int lseek(int fd, int offset, int whence)
{
    long res = syscall3(SYS_lseek, fd, offset, whence);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

_off64_t lseek64(int fd, _off64_t offset, int whence)
{
    _off64_t result;
    long res = syscall5(SYS__llseek, fd, (offset >> 32) & 0xFFFFFFFF, offset & 0xFFFFFFFF, (long)&result, whence);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return result;
}

int open(const char *pathname, int flags, ...)
{
    va_list arg;
    va_start(arg, flags);
    mode_t mode = flags & O_CREAT ? va_arg(arg, int) : 0;
    va_end(arg);
    long res = syscall3(SYS_open, (long)pathname, flags, mode);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

int open64(const char *pathname, int flags, ...)
{
    // the same thing as regular open
    va_list arg;
    va_start(arg, flags);
    mode_t mode = flags & O_CREAT ? va_arg(arg, int) : 0;
    va_end(arg);
    long res = syscall3(SYS_open, (long)pathname, flags, mode);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

int read(int fd, char *buf, int count)
{
    int res = syscall3(SYS_read, fd, (long)buf, count);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return res;
}

caddr_t sbrk(int increment)
{
    uintptr_t cbrk = __current_brk;
    uintptr_t res = syscall1(SYS_brk, (long)(cbrk + increment));
    if(res == ~0u) return (void *)(-1);
    __current_brk = res;
    return (void *)cbrk;
}

int stat(const char *path, struct stat *buf)
{
    long res = syscall2(SYS_stat, (long)path, (long)buf);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

int access(const char *pathname, int mode)
{
    struct stat st;
    int res = stat(pathname, &st);
    if(res < 0) return -1;
    // add proper permission check here
    return 0;
}

clock_t times(struct tms *buf)
{
    fprintf(stderr, "times not implemented\n");
    errno = ENOSYS;
    return (clock_t)-1;
}

int unlink(char *pathname)
{
    long res = syscall1(SYS_unlink, (long)pathname);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

int wait(int *status)
{
    fprintf(stderr, "wait not implemented\n");
    errno = ENOSYS;
    return -1;

}

int write(int fd, char *buf, int count)
{
    int res = syscall3(SYS_write, fd, (long)buf, count);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
}

char *getcwd(char *buf, size_t size)
{
    long res = syscall2(SYS_getcwd, (long)buf, size);
    if(res < 0)
    {
        errno = -res;
        return NULL;
    }
    return buf;
}

int getentropy(void *buffer, size_t length)
{
    // FIXME: I think we could do better ;)
    char *buf = (char *)buffer;
    for(int i = 0; i < length; ++i)
        buf[i] = i + 1;
    return 0;
}

int fcntl(int fd, int cmd, ...)
{
    fprintf(stderr, "%s not implemented\n", __FUNCTION__);
    errno = ENOSYS;
    return -1;
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    return syscall2(SYS_nanosleep, (long)req, (long)rem);
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    void *res = memalign(alignment, size);
    if(!res) return errno;
    if(memptr) *memptr = res;
    return 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    fprintf(stderr, "%s not implemented\n", __FUNCTION__);
    errno = ENOSYS;
    return -1;
}

int mkdir(const char *pathname, mode_t mode)
{
    long res = syscall2(SYS_mkdir, (long)pathname, (long)mode);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

// this goes to time.c

static int isLeap(int year)
{
    if(year % 4) return 0;
    else if(year % 100) return 1;
    else if(year % 400) return 0;
    return 1;
}

static time_t tmToUnix(struct tm *tm)
{
    if(!tm) return 0;

    time_t t;
    time_t y = tm->tm_year;
    time_t m = tm->tm_mon;
    time_t d = tm->tm_mday;

    if(m <= 2)
    {
        m += 12;
        y -= 1;
    }

    t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
    t += (30 * m) + (3 * (m + 1) / 5) + d;
    t -= 719561;
    t *= 86400;
    t += (3600 * tm->tm_hour) + (60 * tm->tm_min) + tm->tm_sec;

    return t;
}

static void unixToTM(time_t t, struct tm *tm)
{
    static const int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if(!tm) return;
    if(t < 0) t = 0; // negative time not supported
    tm->tm_sec = t % 60;
    t /= 60;
    tm->tm_min = t % 60;
    t /= 60;
    tm->tm_hour = t % 24;
    t /= 24;

    time_t a = (4 * t + 102032) / 146097 + 15;
    time_t b = t + 2442113 + a - (a / 4);
    time_t year = (20 * b - 2442) / 7305;
    time_t d = b - 365 * year - (year / 4);
    time_t month = d * 1000 / 30601;
    time_t day = d - month * 30 - month * 601 / 1000;

    if(month <= 13)
    {
        year -= 4716;
        month -= 1;
    }
    else
    {
        year -= 4715;
        month -= 13;
    }

    tm->tm_year = year;
    tm->tm_mon = month;
    tm->tm_mday = day;

    int yday = 0;
    int leap = isLeap(year);
    for(int i = 0; i < month; ++i)
    {
        yday += monthDays[i];
        if(leap && i == 2)
            ++yday;
    }
    yday += day;
    tm->tm_yday = yday;
    tm->tm_wday = (year * 365 + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400) % 7;
}

int gettimeofday(struct timeval *tv, void *tz)
{
    time_t t;
    int res = syscall1(SYS_time, (long)&t);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    if(tv) tv->tv_sec = t;
    return 0;
}

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
    printf("settimeofday not implemented\n");
    errno = ENOSYS;
    return -1;
}

// these go to dirent.c

#define NAME_MAX 255
#define O_DIRECTORY 0200000

/*struct dirent
{
    ino_t d_ino;
    unsigned char d_type;
    char d_name[NAME_MAX + 1];
};*/

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
    if(!dirp)
    {
        errno = EINVAL;
        return NULL;
    }
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

// these go to sofdiv.c

uint64_t __udivmoddi4(uint64_t n, uint64_t d, uint64_t *_r)
{
    if(!d)
    {   // generate division by zero exception
        volatile int a = 1, b = 0, c = 1 / b;
    }

    uint64_t q = 0, r = 0;
    for(int i = 63; i >= 0; --i)
    {
        r = (r << 1) | ((n >> i) & 1);
        if(r >= d)
        {
            r = r - d;
            q |= (1ull << i);
        }
    }
    if(_r) *_r = r;
    return q;
}

int64_t __divmoddi4(int64_t n, int64_t d, int64_t *r)
{
    int64_t sign = 1;
    if(n < 0)
    {
        n = -n;
        sign = -1;
    }
    if(d < 0)
    {
        d = -d;
        sign *= -1;
    }
    int64_t q = __udivmoddi4(n, d, (uint64_t *)r);
    return q * sign;
}

uint64_t __udivdi3(uint64_t n, uint64_t d)
{
    return __udivmoddi4(n, d, 0);
}

int64_t __divdi3(int64_t n, int64_t d)
{
    return __divmoddi4(n, d, 0);
}

uint64_t __umoddi3(uint64_t n, uint64_t d)
{
    uint64_t r;
    __udivmoddi4(n, d, &r);
    return r;
}

int64_t __moddi3(int64_t n, int64_t d)
{
    int64_t r;
    __divmoddi4(n, d, &r);
    return r;
}

// this goes to mman.c

int munmap(void *addr, size_t length)
{
    long res = syscall2(SYS_munmap, (long)addr, length);
    if(res < 0)
    {
        errno = -res;
        return -1;
    }
    return 0;
}

// these to regex.c

typedef void regex_t;
typedef int regoff_t;

typedef struct
{
    regoff_t rm_so;
    regoff_t rm_eo;
} regmatch_t;

int regcomp(regex_t *preg, const char *regex, int cflags)
{
    fprintf(stderr, "%s not implemented\n", __FUNCTION__);
    return 0;
}

int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags)
{
    fprintf(stderr, "%s not implemented\n", __FUNCTION__);
    return 0;
}

size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size)
{
    fprintf(stderr, "%s not implemented\n", __FUNCTION__);
    return 0;
}

void regfree(regex_t *preg)
{
    fprintf(stderr, "%s not implemented\n", __FUNCTION__);
}
