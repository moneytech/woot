#include <types.h>

typedef      int si_int;
typedef unsigned su_int;

typedef          long long di_int;
typedef unsigned long long du_int;

#define UINT64_C(c) c ## ULL

struct DIstruct
{
    int32_t high, low;
};

typedef union
{
    struct DIstruct s;
    int64_t ll;
} DIunion;

void __divide_error();
int64_t __lshrdi3 (int64_t u, uint16_t b);
int64_t __ashldi3 (int64_t u, uint16_t b);
int64_t __ashrdi3 (int64_t u, uint16_t b);
int64_t __divdi3(int64_t num, int64_t den);
uint64_t __udivdi3(uint64_t num, uint64_t den);

int64_t __moddi3(int64_t num, int64_t den);
uint64_t __umoddi3(uint64_t num, uint64_t den);

int64_t __divmoddi4(int64_t num, int64_t den, int64_t *rem_p);
uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t *rem_p);

int64_t __moddi3(int64_t num, int64_t den)
{
    int64_t v;
    __divmoddi4(num, den, &v);
    return v;
}

int64_t __divmoddi4(int64_t num, int64_t den, int64_t *rem_p)
{
    int minus = 0;
    int64_t v;

    if ( num < 0 ) {
        num = -num;
        minus = 1;
    }
    if ( den < 0 ) {
        den = -den;
        minus ^= 1;
    }

    v = __udivmoddi4(num, den, (uint64_t *)rem_p);
    if ( minus ) {
        v = -v;
        if ( rem_p )
            *rem_p = -(*rem_p);
    }

    return v;
}

int64_t __lshrdi3 (int64_t u, uint16_t b)
{
    DIunion w;
    uint16_t bm;
    DIunion uu;

    if(b == 0)
        return u;

    uu.ll = u;

    bm = (sizeof(int64_t) * 8) - b;
    if (bm <= 0)
    {
        w.s.high = 0;
        w.s.low = (uint32_t)uu.s.high >> -bm;
    }
    else
    {
        uint32_t carries = (uint32_t)uu.s.high << bm;
        w.s.high = (uint32_t)uu.s.high >> b;
        w.s.low = ((uint32_t)uu.s.low >> b) | carries;
    }

    return w.ll;
}

int64_t __ashldi3 (int64_t u, uint16_t b)
{
    DIunion w;
    uint16_t bm;
    DIunion uu;

    if(b == 0)
        return u;

    uu.ll = u;

    bm = (sizeof(int32_t) * 8) - b;
    if (bm <= 0)
    {
        w.s.low = 0;
        w.s.high = (uint32_t)uu.s.low << -bm;
    }
    else
    {
        uint32_t carries = (uint32_t)uu.s.low >> bm;
        w.s.low = (uint32_t)uu.s.low << b;
        w.s.high = ((uint32_t)uu.s.high << b) | carries;
    }

    return w.ll;
}

int64_t __ashrdi3 (int64_t u, uint16_t b)
{
    DIunion w;
    uint16_t bm;
    DIunion uu;

    if(b == 0)
        return u;

    uu.ll = u;

    bm = (sizeof(int32_t) * 8) - b;
    if (bm <= 0)
    {
        /* w.s.high = 1..1 or 0..0 */
        w.s.high = uu.s.high >> (sizeof(int32_t) * 8 - 1);
        w.s.low = uu.s.high >> -bm;
    }
    else
    {
        uint32_t carries = (uint32_t)uu.s.high << bm;
        w.s.high = uu.s.high >> b;
        w.s.low = ((uint32_t)uu.s.low >> b) | carries;
    }

    return w.ll;
}

int64_t __divdi3(int64_t num, int64_t den)
{
    int minus = 0;
    int64_t v;

    if(num < 0)
    {
        num = -num;
        minus = 1;
    }
    if(den < 0)
    {
        den = -den;
        minus ^= 1;
    }

    v = __udivmoddi4(num, den, nullptr);
    if(minus)
        v = -v;

    return v;
}

uint64_t __udivdi3(uint64_t num, uint64_t den)
{
    return __udivmoddi4(num, den, nullptr);
}

uint64_t __umoddi3(uint64_t num, uint64_t den)
{
    uint64_t v = 0;
    __udivmoddi4(num, den, &v);
    return v;
}

uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t *rem_p)
{
    uint64_t quot = 0, qbit = 1;

    if(den == 0)
    {
        __divide_error();
        return 0;/* If trap returns... */
    }

    /* Left-justify denominator and count shift */
    while((int64_t)den >= 0)
    {
        den <<= 1;
        qbit <<= 1;
    }

    while(qbit)
    {
        if(den <= num)
        {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    if(rem_p)
        *rem_p = num;

    return quot;
}

void __divide_error()
{
    asm("int $0");
}

double __floatundidf(du_int a)
{
    static const double twop52 = 4503599627370496.0; // 0x1.0p52
    static const double twop84 = 19342813113834066795298816.0; // 0x1.0p84
    static const double twop84_plus_twop52 = 19342813118337666422669312.0; // 0x1.00000001p84

    union { uint64_t x; double d; } high = { .d = twop84 };
    union { uint64_t x; double d; } low = { .d = twop52 };

    high.x |= a >> 32;
    low.x |= a & UINT64_C(0x00000000ffffffff);

    const double result = (high.d - twop84_plus_twop52) + low.d;
    return result;
}

long long __tcc_cvt_ftol(long double x)
{
    unsigned c0, c1;
    long long ret;
    __asm__ __volatile__ ("fnstcw %0" : "=m" (c0));
    c1 = c0 | 0x0C00;
    __asm__ __volatile__ ("fldcw %0" : : "m" (c1));
    __asm__ __volatile__ ("fistpll %0"  : "=m" (ret));
    __asm__ __volatile__ ("fldcw %0" : : "m" (c0));
    return ret;
}
