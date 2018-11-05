#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("arguments:\n");
    for(int i = 0; i < argc; ++i)
        printf("arg: %d val: %s\n", i, argv[i]);
    printf("end of arguments\n");

    char buf[128];
    printf("type something > ");
    int br = fread(buf, 1, sizeof(buf), stdin);
    char *nl = strrchr(buf, '\n');
    if(nl) *nl = 0;
    printf("you typed: '%*s'\n", br, buf);

    return 42;
}
