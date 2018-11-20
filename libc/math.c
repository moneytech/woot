#include <math.h>
#include <stdint.h>

static double square(double x)
{
    return x * x;
}

double ceil(double x)
{
    return -floor(-x);
}

float ceilf(float x)
{
    return -floorf(-x);
}

long double ceill(long double x)
{
    return -floorl(-x);
}

double fabs(double x)
{
    return x < 0 ? -x : x;
}

float fabsf(float x)
{
    return x < 0 ? -x : x;
}

long double fabsl(long double x)
{
    return x < 0 ? -x : x;
}

double floor(double x)
{
    int xi = (int)x;
    return x < xi ? xi - 1 : xi;
}

float floorf(float x)
{
    int xi = (int)x;
    return x < xi ? xi - 1 : xi;
}

long double floorl(long double x)
{
    int xi = (int)x;
    return x < xi ? xi - 1 : xi;
}

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


double modf(double x, double *intpart)
{
    double mod = fmod(x, 1.0);
    *intpart = x - mod;
    return mod;
}

float modff(float x, float *intpart)
{
    float mod = fmodf(x, 1.0);
    *intpart = x - mod;
    return mod;
}

long double modfl (long double x, long double *intpart)
{
    long double mod = fmodl(x, 1.0);
    *intpart = x - mod;
    return mod;
}

// taken from dietlibc
// TODO: create own implementation

double pow(double base, double exponent)
{
    unsigned int e;
    long double ret;

    if(base == 0.0)
    {
        if(exponent > 0.0) return 0.0;
        else if(exponent == 0.0) return 1.0;
        else return 1.0 / base;
    }

    if(exponent == (int)(e = (int)exponent))
    {
        if((int)e < 0)
        {
            e = -e;
            base = 1.0 / base;
        }

        ret = 1.0;

        while(1)
        {
            if(e &1) ret *= base;
            if((e >>= 1) == 0)
                break;
            base *= base;
        }
        return ret;
    }

    return exp(log(base) * exponent);
}

