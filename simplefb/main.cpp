#include <errno.h>
#include <multiboot.h>
#include <simplefb.h>
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
    printf("[simplefb] FrameBuffer address: %#.8x\n"
           "           width: %d\n"
           "           height: %d\n"
           "           bpp: %d\n",
           (uintptr_t)MultibootInfo->framebuffer_addr, MultibootInfo->framebuffer_width,
           MultibootInfo->framebuffer_height, MultibootInfo->framebuffer_bpp);
    SimpleFB *fb = new SimpleFB((void *)(uintptr_t)MultibootInfo->framebuffer_addr,
                                MultibootInfo->framebuffer_width, MultibootInfo->framebuffer_height,
                                MultibootInfo->framebuffer_bpp, MultibootInfo->framebuffer_pitch,
                                MultibootInfo->framebuffer_red_field_position,
                                MultibootInfo->framebuffer_green_field_position,
                                MultibootInfo->framebuffer_blue_field_position,
                                MultibootInfo->framebuffer_red_mask_size,
                                MultibootInfo->framebuffer_green_mask_size,
                                MultibootInfo->framebuffer_blue_mask_size);
    fb->Pixels->Clear(PixMap::Color(48, 64, 16));
    return FrameBuffer::Add(fb);
}

extern "C" void Cleanup()
{
    printf("[simplefb] Cleanup()\n");
}

