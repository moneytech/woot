#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

void *memset(void *dst, int val, size_t n)
{
    if(!n) return dst;
    byte *buf = (byte *)dst;
    while(n--)
        *buf++ = val;
    return dst;
}

void *wmemset(void *dst, int val, size_t n)
{
    if(!n) return dst;
    word *buf = (word *)dst;
    while(n--)
        *buf++ = val;
    return dst;
}

void *lmemset(void *dst, int val, size_t n)
{
    if(!n) return dst;
    dword *buf = (dword *)dst;
    while(n--)
        *buf++ = val;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    byte *d = (byte *)dst;
    byte *s = (byte *)src;
    if(!n || dst == src)
        return dst;
    else if(src > dst)
    {
        while(n--) *d++ = *s++;
        return dst;
    }
    d += n;
    s += n;
    while(n--) *(--d) = *(--s);
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    return memmove(dst, src, n);
}

void *bltcpy(void *dst, const void *src, size_t bpl, size_t stride, size_t lines)
{
    byte *d = (byte *)dst;
    byte *s = (byte *)src;
    while(lines--)
    {
        memmove(d, s, bpl);
        d += stride;
        s += stride;
    }
    return dst;
}

void *bltmove(void *dst, const void *src, size_t bpl, size_t stride, size_t lines)
{
    byte *d = (byte *)dst;
    byte *s = (byte *)src;
    bool fwd = d < s;

    if(fwd)
    {
        while(lines--)
        {
            memmove(d, s, bpl);
            d += stride;
            s += stride;
        }
    }
    else
    {
        d += stride * lines;
        s += stride * lines;
        while(lines--)
        {
            d -= stride;
            s -= stride;
            memmove(d, s, bpl);
        }
    }
    return dst;
}

int memcmp(const void *m1, const void *m2, size_t n)
{
    byte *s1 = (byte *)m1;
    byte *s2 = (byte *)m2;
    byte u1, u2;

    for (;n-- ;s1++, s2++)
    {
        u1 = *(byte *)s1;
        u2 = *(byte *)s2;
        if(u1 != u2)
            return u1 - u2;
    }
    return 0;
}

size_t strlen(const char *str)
{
    if(!str)
        return 0;

    const char *s;

    for (s = str; *s; ++s);
    return(s - str);
}

int strcmp(const char *s1, const char *s2)
{
    byte a, b;
    do
    {
        a = *s1++;
        b = *s2++;
        byte d = a - b;
        if(d) return d;
    } while(a && b);
    return 0;
}

int strcasecmp(const char *s1, const char *s2)
{
    byte a, b;
    do
    {
        a = tolower(*s1++);
        b = tolower(*s2++);
        byte d = a - b;
        if(d) return d;
    } while(a && b);
    return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    for ( ; n > 0; s1++, s2++, --n) if (*s1 != *s2) return ((*(byte *)s1 < *(byte *)s2) ? -1 : +1);
    else if (*s1 == '\0') return 0;
    return 0;
}

size_t wcslen(const wchar_t *str)
{
    if(!str)
        return 0;
    int i;
    for(i = 0; *str++; i++);
    return i;
}

char *strdup(const char *s)
{
    if(!s) return nullptr;
    int len = strlen(s);
    char *str = (char *)malloc(len + 1);
    memcpy(str, s, len + 1);
    return str;
}

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;
    while(*dest++ = *src++);
    return ret;
}

char *strncpy(char *dst, const char *src, size_t n)
{
   char *temp = dst;
   while (n-- && (*dst++ = *src++));
   return temp;
}

char *strchr(const char *s, int c)
{
    while(*s != (char)c)
    {
        if(!*s++)
            return 0;
    }
    return (char *)s;
}

char *strrchr(const char *s, int c)
{
    char* ret=0;
    do
    {
        if(*s == (char)c)
            ret = (char *)s;
    } while(*s++);
    return ret;
}

size_t strspn(const char *s1, const char *s2)
{
    size_t ret=0;
    while(*s1 && strchr(s2, *s1++))
        ret++;
    return ret;
}

size_t strcspn(const char *s1, const char *s2)
{
    size_t ret=0;
    while(*s1)
    {
        if(strchr(s2, *s1)) return ret;
        else s1++, ret++;
    }
    return ret;
}

char *strcat(char *dest, const char *src)
{
    char *ret = dest;
    while (*dest)
        dest++;
    while(*dest++ = *src++);
    return ret;
}

char *strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;
    if(!str) str = *nextp;
    str += strspn(str, delim);
    if(*str == '\0') return nullptr;
    ret = str;
    str += strcspn(str, delim);
    if(*str) *str++ = '\0';
    *nextp = str;
    return ret;
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int tolower(int c)
{
    if(c >= 'A' && c <= 'Z')
        return c + 32;
    return c;
}

int toupper(int c)
{
    if(c >= 'a' && c <= 'z')
        return c - 32;
    return c;
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

int isxdigit(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int isalpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int isupper(int c)
{
    return (c >= 'A' && c <= 'Z');
}

long strtol(const char *nptr, char **endptr, int base)
{
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    } else if ((base == 0 || base == 2) &&
        c == '0' && (*s == 'b' || *s == 'B')) {
        c = s[1];
        s += 2;
        base = 2;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
        errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

unsigned long strtoul(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    unsigned long acc;
    int c;
    unsigned long cutoff;
    int neg = 0, any, cutlim;

    do
    {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    } else if ((base == 0 || base == 2) &&
        c == '0' && (*s == 'b' || *s == 'B')) {
        c = s[1];
        s += 2;
        base = 2;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
    cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULONG_MAX;
        errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

unsigned long long strtoull(const char *nptr, char **endptr, int base)
{
    const char *s;
    unsigned long long acc;
    char c;
    unsigned long long cutoff;
    int neg, any, cutlim;

    s = nptr;
    do {
        c = *s++;
    } while (isspace((unsigned char)c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    acc = any = 0;
    if (base < 2 || base > 36)
        goto noconv;

    cutoff = ULLONG_MAX / base;
    cutlim = ULLONG_MAX % base;
    for ( ; ; c = *s++) {
        if (c >= '0' && c <= '9')
            c -= '0';
        else if (c >= 'A' && c <= 'Z')
            c -= 'A' - 10;
        else if (c >= 'a' && c <= 'z')
            c -= 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULLONG_MAX;
    } else if (!any) {
noconv:
        acc = ULLONG_MAX;
        errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != NULL)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

static int maxExponent = 511;
static double powersOf10[] =
{
    10.,
    100.,
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};

double strtod(const char *string, char **endPtr)
{
    int sign, expSign = FALSE;
    double fraction, dblExp, *d;
    register const char *p;
    register int c;
    int exp = 0;		/* Exponent read from "EX" field. */
    int fracExp = 0;		/* Exponent that derives from the fractional
                 * part.  Under normal circumstatnces, it is
                 * the negative of the number of digits in F.
                 * However, if I is very long, the last digits
                 * of I get dropped (otherwise a long I with a
                 * large negative exponent could cause an
                 * unnecessary overflow on I alone).  In this
                 * case, fracExp is incremented one for each
                 * dropped digit. */
    int mantSize;		/* Number of digits in mantissa. */
    int decPt;			/* Number of mantissa digits BEFORE decimal
                 * point. */
    const char *pExp;		/* Temporarily holds location of exponent
                 * in string. */

    /*
     * Strip off leading blanks and check for a sign.
     */

    p = string;
    while (isspace(*p)) {
        p += 1;
    }
    if (*p == '-') {
        sign = TRUE;
        p += 1;
    } else {
        if (*p == '+') {
            p += 1;
        }
        sign = FALSE;
    }

    /*
     * Count the number of digits in the mantissa (including the decimal
     * point), and also locate the decimal point.
     */

    decPt = -1;
    for (mantSize = 0; ; mantSize += 1)
    {
        c = *p;
        if (!isdigit(c)) {
            if ((c != '.') || (decPt >= 0)) {
                break;
            }
            decPt = mantSize;
        }
        p += 1;
    }

    /*
     * Now suck up the digits in the mantissa.  Use two integers to
     * collect 9 digits each (this is faster than using floating-point).
     * If the mantissa has more than 18 digits, ignore the extras, since
     * they can't affect the value anyway.
     */

    pExp  = p;
    p -= mantSize;
    if (decPt < 0) {
        decPt = mantSize;
    } else {
        mantSize -= 1;			/* One of the digits was the point. */
    }
    if (mantSize > 18) {
        fracExp = decPt - 18;
        mantSize = 18;
    } else {
        fracExp = decPt - mantSize;
    }
    if (mantSize == 0) {
        fraction = 0.0;
        p = string;
        goto done;
    } else {
        int frac1, frac2;
        frac1 = 0;
        for ( ; mantSize > 9; mantSize -= 1)
        {
            c = *p;
            p += 1;
            if (c == '.') {
                c = *p;
                p += 1;
            }
            frac1 = 10*frac1 + (c - '0');
        }
        frac2 = 0;
        for (; mantSize > 0; mantSize -= 1)
        {
            c = *p;
            p += 1;
            if (c == '.') {
                c = *p;
                p += 1;
            }
            frac2 = 10*frac2 + (c - '0');
        }
        fraction = (1.0e9 * frac1) + frac2;
    }

    /*
     * Skim off the exponent.
     */

    p = pExp;
    if ((*p == 'E') || (*p == 'e')) {
        p += 1;
        if (*p == '-') {
            expSign = TRUE;
            p += 1;
        } else {
            if (*p == '+') {
                p += 1;
            }
            expSign = FALSE;
        }
        if (!isdigit(*p)) {
            p = pExp;
            goto done;
        }
        while (isdigit(*p)) {
            exp = exp * 10 + (*p - '0');
            p += 1;
        }
    }
    if (expSign) {
        exp = fracExp - exp;
    } else {
        exp = fracExp + exp;
    }

    /*
     * Generate a floating-point number that represents the exponent.
     * Do this by processing the exponent one bit at a time to combine
     * many powers of 2 of 10. Then combine the exponent with the
     * fraction.
     */

    if (exp < 0) {
        expSign = TRUE;
        exp = -exp;
    } else {
        expSign = FALSE;
    }
    if (exp > maxExponent) {
        exp = maxExponent;
        errno = ERANGE;
    }
    dblExp = 1.0;
    for (d = powersOf10; exp != 0; exp >>= 1, d += 1) {
        if (exp & 01) {
            dblExp *= *d;
        }
    }
    if (expSign) {
        fraction /= dblExp;
    } else {
        fraction *= dblExp;
    }

done:
    if (endPtr != NULL) {
        *endPtr = (char *) p;
    }

    if (sign) {
        return -fraction;
    }
    return fraction;
}

char *strrand(char *buffer, size_t nChars)
{
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for(int i = 0; i < nChars; ++i)
        buffer[i] = charset[rand() % sizeof(charset)];
    buffer[nChars] = 0;
    return buffer;
}
