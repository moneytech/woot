#include <stdlib.h>
#include <sys/syscall.h>
#include <test.h>
#include <unistd.h>

#include "../libc/internal/syscall.h"

int main(int argc, char *argv[])
{
    char testText[] = "trolololo\n";
    write(0, testText, sizeof(testText) - 1);
    syscall0(23);

    asm("int $0x80":: "a"(123), "b"(2000));
    asm("int $0x80":: "a"(13));

    exit(42);
}
