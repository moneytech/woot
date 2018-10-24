extern "C" void _start()
{
    asm("int $0x80":: "a"(12));
    asm("int $0x80":: "a"(123));
    asm("int $0x80":: "a"(13));
}
