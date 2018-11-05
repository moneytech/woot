#include <inttypes.h>
#include <stdlib.h>

int64_t __divmoddi4(int64_t n, int64_t d, int64_t *r);

intmax_t imaxabs(intmax_t n)
{
    return n < 0 ? -n : n;
}

imaxdiv_t imaxdiv(intmax_t n, intmax_t d)
{
    imaxdiv_t r;
    r.quot = __divmoddi4(n, d, &r.rem);
    return r;
}

intmax_t strtoimax(const char *str, char **endptr, int base)
{
    return strtoll(str, endptr, base);
}

uintmax_t strtoumax(const char *str, char **endptr, int base)
{
    return strtoull(str, endptr, base);
}
