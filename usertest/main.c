#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <test.h>
#include <time.h>
#include <unistd.h>

#include "../libc/internal/syscall.h"

int main(int argc, char *argv[])
{
    char testText[] = "Trololololo...\n";
    write(0, testText, sizeof(testText) - 1);

    for(int i = 0; i < 20; ++i)
        printf("%s9000 %2d %#.8x %.5f\n", "over", i, i * 1000, 1.0 / 3);

    return 42;
}
