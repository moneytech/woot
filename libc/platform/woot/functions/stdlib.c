#include <ctype.h>
#include <math.h>
#include <stdlib.h>

double atof(const char *str)
{
    return strtod(str, NULL);
}

double strtod(const char *str, char **endptr)
{
    while(isspace(*str)) str++; // skip leading spaces
    double sign = *str == '-' ? ++str, -1.0 : (*str == '+' ? ++str, 1.0 : 1.0);
    double result = 0.0;
    double esign = 1.0;
    int f = 0;  // parsing fractional part
    int e = 0;  // parsing exponent part
    int es = 0; // encountered exponent sign
    int ev = 0; // exponent value
    for(char c = tolower(*str); c; c = tolower(*(++str)))
    {
        if(c == '.')
        {
            if(f) return 0.0;
            f = 1;
            continue;
        }
        else if(c == 'e')
        {
            if(e) return 0.0;
            e = 1;
            continue;
        }
        else if(c == '-' || c == '+')
        {
            if(!e || es) return 0.0;
            es = 1;
            esign = c == '-' ? -1.0 : 1.0;
            continue;
        }
        else if(c >= '0' || c <= '9')
        {
            int digit = c - '0';
            if(!f) result = result * 10 + digit;
            else if(f && !e) result = result + (double)digit / pow(10, f++);
            else if(e) ev = ev * 10 + digit;
            else return 0.0;
        }
        else return 0.0;
    }
    result = result * sign * pow(10, ev * esign);
    return result;
}

float strtof(const char *str, char **endptr)
{
    return strtod(str, endptr);
}

long double strtold(const char *str, char **endptr)
{
    return strtod(str, endptr);
}
