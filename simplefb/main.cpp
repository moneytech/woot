#include <stdio.h>

extern "C" int Initialize()
{
    printf("[simplefb] Initialize()\n");
	return 0;
}

extern "C" void Cleanup()
{
    printf("[simplefb] Cleanup()\n");
}

