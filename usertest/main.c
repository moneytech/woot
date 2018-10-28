#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char testText[] = "Trololololo...\n";
    write(0, testText, sizeof(testText) - 1);

    char cwd[256];
    printf("current directory: '%s'\n", getcwd(cwd, sizeof(cwd)));

    const char *filename = "modulelist";
    FILE *f = fopen(filename, "rb");
    if(f)
    {
        for(char buf[16]; !feof(f);)
        {
            size_t n = fread(buf, 1, sizeof(buf), f);
            if(ferror(f))
            {
                printf("read error\n");
                break;
            }
            printf("%.*s", (int)n, buf);
        }
        fclose(f);
    } else printf("couldn't open '%s'\n", filename);

    for(int i = 0; i < 5; ++i)
    {
        printf("%s9000 %2d %#.8x %.5f\n", "over", i, i * 1000, 1.0 / 3);
        struct timespec t = { 0, 500 * 1000000 };
        nanosleep(&t, NULL);
    }

    return 42;
}
