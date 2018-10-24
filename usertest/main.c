#include <stdlib.h>
#include <sys/syscall.h>
#include <test.h>
#include <time.h>
#include <unistd.h>

#include "../libc/internal/syscall.h"

int main(int argc, char *argv[])
{
    char testText[] = "Wait for 5 seconds...\n";
    write(0, testText, sizeof(testText) - 1);

    syscall0(23);
    struct timespec ts = { 5, 0 };
    struct timespec tr;
    nanosleep(&ts, &tr);
    asm("int $0x80":: "a"(13));

    exit(42);
}
