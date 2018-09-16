extern "C" int kmain(void *mbootInfo)
{
    unsigned short *video = (unsigned short *)0xB8000;
    video[0] = 0x1F00 | 'X';
    return 0xD007D007;
}
