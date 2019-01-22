#include <es1371.h>
#include <stdio.h>

extern "C" int Initialize()
{
    printf("[es1371] Initialize()\n");
    ES1371::Initialize();
    return 0;
}

extern "C" void Cleanup()
{
    printf("[es1371] Cleanup()\n");
    ES1371::Cleanup();
}
