#include <cmi8738.h>
#include <stdio.h>

extern "C" int Initialize()
{
    printf("[cmi8738] Initialize()\n");
    CMI8738::Initialize();
    return 0;
}

extern "C" void Cleanup()
{
    printf("[cmi8738] Cleanup()\n");
    CMI8738::Cleanup();
}
