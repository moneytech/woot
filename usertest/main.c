#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    for(int i = 0; i < argc; ++i)
        printf("arg: %d val: %s\n", i, argv[i]);

    char testText[] = "Trololololo...\n";
    write(1, testText, sizeof(testText) - 1);

    char cwd[256];
    printf("current directory: '%s'\n", getcwd(cwd, sizeof(cwd)));

    const char *filename = "modulelist";
    struct stat st;
    if(!stat(filename, &st))
    {
        printf("st_dev: %d\n", (int)st.st_dev);
        printf("st_ino: %d\n", (int)st.st_ino);
        printf("st_size: %d\n", (int)st.st_size);
        printf("st_mode: %#o (%s)\n", (int)(st.st_mode & 07777), S_ISDIR(st.st_mode) ? "dir" : "file");
    }
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

    return 42;
}
