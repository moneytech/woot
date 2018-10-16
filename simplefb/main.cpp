#include <errno.h>
#include <multiboot.h>
#include <stdio.h>
#include <types.h>

extern multiboot_info_t *MultibootInfo;

extern "C" int Initialize()
{
    printf("[simplefb] Initialize()\n");
    if(MultibootInfo->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB)
    {
        printf("[simplefb] MultibootInfo->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB\n");
        return -EINVAL;
    }
    uintptr_t fbPtr = MultibootInfo->framebuffer_addr;
    printf("[simplefb] Framebuffer address: %#.8x\n", fbPtr);
	return 0;
}

extern "C" void Cleanup()
{
    printf("[simplefb] Cleanup()\n");
}

