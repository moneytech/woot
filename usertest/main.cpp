extern "C" void _start()
{
    asm("int $0x80":: "a"(12));
    asm("int $0x80":: "a"(123), "b"(2000));
    //asm("outb %%al, %%dx":: "a"('U'), "d"(0x3F8));
    asm("int $0x80":: "a"(13));
}
