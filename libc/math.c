#include <math.h>
#include <stdint.h>

int isinf(double x)
{
    union
    {
        uint64_t u;
        double f;
    } val;
    val.f = x;
    return ((val.u >> 32) & 0x7fffffff) == 0x7ff00000 && (val.u == 0);
}

int isnan(double x)
{
    union
    {
        uint64_t u;
        double f;
    } val;
    val.f = x;
    return ((val.u >> 32) & 0x7fffffff) + (val.u != 0) > 0x7ff00000;
}
