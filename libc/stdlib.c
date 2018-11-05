#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

static int rsx = 0x1BADB002, rsy = 0xBAADF00D, rsz = 0xB16B00B2, rsw = 0x10121337;

void srand(int seed)
{
    // FIXME: needs lock
    rsx = 0x1BADB002 ^ seed;
    rsy = 0xBAADF00D ^ seed;
    rsz = 0xB16B00B2 ^ seed;
    rsw = 0x10121337 ^ seed;
}

int rand()
{
    // FIXME: needs lock
    int t = rsx ^ (rsx << 11);
    rsx = rsy; rsy = rsz; rsz = rsw;
    rsw = rsw ^ (rsw >> 19) ^ t ^ (t >> 8);
    return rsw;
}

void exit(int status)
{
    _exit(status);
}

void abort()
{
    _exit(127);
}

char *getenv(const char *name)
{
    // Not implemented yet
    return NULL;
}

long strtol(const char *str, char **endptr, int base)
{
    while(isspace(*str)) str++; // skip leading spaces
    long sign = *str == '-' ? ++str, -1 : (*str == '+' ? ++str, 1 : 1);
    if((!base || base == 16) && *str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        base = 16;
    }
    else if((!base || base == 2) && *str == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        str += 2;
        base = 2;
    }
    if(!base)
        base = *str == '0' ? 8 : 10; // select default base
    long result = 0;
    for(char c = tolower(*str); c; c = tolower(*(++str)))
    {
        int digit = 0;
        if(c >= '0' && c <= '9')
            digit = c - '0';
        else if(c >= 'a' && c <= 'z')
            digit = c - 'a';
        else break;
        if(digit >= base)
            break;
        long val = result * base + digit * sign;
        if(sign > 0)
        {
            if(val < result)
            {
                errno = ERANGE;
                result = LONG_MAX;
                break;
            }
        }
        else
        {
            if(val > result)
            {
                errno = ERANGE;
                result = LONG_MIN;
                break;
            }
        }
        result = val;
    }
    if(endptr) *endptr = (char *)str;
    return result;
}

unsigned long strtoul(const char *str, char **endptr, int base)
{
    return (unsigned long)strtol(str, endptr, base);
}

long long strtoll(const char *str, char **endptr, int base)
{
    while(isspace(*str)) str++; // skip leading spaces
    long long sign = *str == '-' ? ++str, -1 : (*str == '+' ? ++str, 1 : 1);
    if((!base || base == 16) && *str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        base = 16;
    }
    else if((!base || base == 2) && *str == '0' && (str[1] == 'b' || str[1] == 'B'))
    {
        str += 2;
        base = 2;
    }
    if(!base)
        base = *str == '0' ? 8 : 10; // select default base
    long long result = 0;
    for(char c = tolower(*str); c; c = tolower(*(++str)))
    {
        int digit = 0;
        if(c >= '0' && c <= '9')
            digit = c - '0';
        else if(c >= 'a' && c <= 'z')
            digit = c - 'a';
        else break;
        if(digit >= base)
            break;
        long long val = result * base + digit * sign;
        if(sign > 0)
        {
            if(val < result)
            {
                errno = ERANGE;
                result = LLONG_MAX;
                break;
            }
        }
        else
        {
            if(val > result)
            {
                errno = ERANGE;
                result = LLONG_MIN;
                break;
            }
        }
        result = val;
    }
    if(endptr) *endptr = (char *)str;
    return result;
}

unsigned long long strtoull(const char *str, char **endptr, int base)
{
    return (unsigned long long)strtoll(str, endptr, base);
}

int atoi(const char *str)
{
    return strtol(str, NULL, 10);
}
