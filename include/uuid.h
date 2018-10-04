#ifndef UUID_H
#define UUID_H

#include <types.h>

class UUID
{
public:
#pragma pack(push, 1)
    union
    {
        struct
        {
            uint32_t time_low;
            uint16_t time_mid;
            uint16_t time_hi_and_version;
            uint16_t clock_seq_hi_and_res_clock_seq_low;
            uint8_t node[6];
        };
        uint8_t Data[16];
    };
#pragma pack(pop)
    static UUID nil;

    UUID();
    UUID(void *data);
    UUID(const char *str);
    UUID(UUID &src);
    void ToString(char *buffer);
};

#endif // UUID_H
