#include <ps2mouse.h>
#include <stdio.h>
#include <types.h>

extern "C" int Initialize()
{
    printf("[ps2mouse] Initialize()\n");
    PS2Mouse::Initialize();
    return 0;
}

extern "C" void Cleanup()
{
    printf("[ps2mouse] Cleanup()\n");
    PS2Mouse::Cleanup();
}

