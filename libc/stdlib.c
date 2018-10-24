#include <stdlib.h>
#include <unistd.h>

static int rsx = 0x1BADB002, rsy = 0xBAADF00D, rsz = 0xB16B00B2, rsw = 0x10121337;

void srand(int seed)
{
    // FIXME: needs lock
    rsx = 0x1BADB002 ^ seed;
    rsy = 0xBAADF00D ^ seed;
    rsz = 0xB16B00B2 ^ seed;
    rsw = 0x10121337 ^ seed;
}

int rand()
{
    // FIXME: needs lock
    int t = rsx ^ (rsx << 11);
    rsx = rsy; rsy = rsz; rsz = rsw;
    rsw = rsw ^ (rsw >> 19) ^ t ^ (t >> 8);
    return rsw;
}

void exit(int status)
{
    _exit(status);
}
