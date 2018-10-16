#include <math.h>
#include <stdlib.h>
#include <stream.h>
#include <string.h>
#include <uuid.h>

const static char *decTable = "0123456789";
const static uint64_t pow10Table[] =
{
    1ull,
    10ull,
    100ull,
    1000ull,
    10000ull,
    100000ull,
    1000000ull,
    10000000ull,
    100000000ull,
    1000000000ull,
    10000000000ull,
    100000000000ull,
    1000000000000ull,
    10000000000000ull,
    100000000000000ull,
    1000000000000000ull,
    10000000000000000ull,
    100000000000000000ull,
    1000000000000000000ull,
    10000000000000000000ull
};

char Stream::ReadChar()
{
    char c;
    int64_t br = Read(&c, 1);
    if(br <= 0) return 0;
    return c;
}

int64_t Stream::ReadLine(char *buffer, int64_t maxLen)
{
    if(!maxLen)
        return 0;
    int64_t i;
    for(i = 0; i < maxLen - 1;)
    {
        char c = ReadChar();
        if(c == 127)
        { // backspace
            if(i) i--;
            continue;
        }
        else if(!c)
        {
            buffer[i] = c;
            return i;
        }
        else if(c == '\n' || c == '\r')
            break;
        buffer[i++] = c;
    }
    buffer[i++] = 0;
    return i;
}

int64_t Stream::WriteStr(const char *str, int64_t maxLen)
{
    int64_t i = strlen(str);
    if(maxLen && i > maxLen) i = maxLen;
    return Write(str, i);
}

int64_t Stream::WriteHex(uint64_t value, bool caps, uintn minDigits, uintn maxDigits, bool measure)
{
    const static char *hexTableLo = "0123456789abcdef";
    const static char *hexTableUp = "0123456789ABCDEF";

    if(minDigits < 1)
        minDigits = 1;
    else if(minDigits > 16)
        minDigits = 16;
    if(maxDigits < 1)
        maxDigits = 1;
    else if(minDigits > 16)
        maxDigits = 16;
    if(minDigits > maxDigits)
        maxDigits = minDigits;

    char buf[20] = { 'X' };
    bool nonzero = false;
    int i, j;
    for(i = 0, j = 0; i < 16; i++, value <<= 4)
    {
        int dig = (value >> 60) & 0x0F;
        nonzero |= dig != 0;
        char c = caps ? hexTableUp[dig] : hexTableLo[dig];
        if((16 - i <= maxDigits && nonzero) || 16 - i <= minDigits)
            buf[j++] = c;
    }
    buf[j] = 0;
    return measure ? strlen(buf) : WriteStr(buf, 0);

}

int64_t Stream::WriteDec(uint64_t value, uintn minDigits, uintn maxDigits, bool showPlus, bool measure, bool unsig)
{
    if(minDigits < 1)
        minDigits = 1;
    else if(minDigits > 19)
        minDigits = 19;
    if(maxDigits < 1)
        maxDigits = 1;
    else if(minDigits > 19)
        maxDigits = 19;
    if(minDigits > maxDigits)
        maxDigits = minDigits;

    char buf[24] = { 'X' };
    bool nonzero = false;
    int i, j = 0;
    int64_t d = 1000000000000000000;
    if(showPlus && value >= 0)
        buf[j++] = '+';
    else if(!unsig && ((int64_t)value) < 0)
    {
        value = -value;
        buf[j++] = '-';
    }
    for(i = 0; i < 19 && d != 0; i++, d /= 10)
    {
        int dig = value / d;
        value -= dig * d;
        nonzero |= dig != 0;
        char c = decTable[dig % 10];
        if((19 - i <= maxDigits && nonzero) || 19 - i <= minDigits)
            buf[j++] = c;
    }
    buf[j] = 0;
    return measure ? strlen(buf) : WriteStr(buf, 0);
}

int64_t Stream::WriteFmt(const char *fmt, ...)
{
    int64_t r = 0;
    va_list args;
    va_start(args, fmt);
    r = VWriteFmt(fmt, args);
    va_end(args);
    return r;
}

int64_t Stream::VWriteFmt(const char *fmt, va_list args)
{
    bool hashFlag = false;
    bool dot = false;
    int width = 0;
    int precision = 0;
    bool specifier = false;
    bool argWidth = false;
    bool leftJustify = false;
    bool zeroPad = false;
    char padChr = ' ';
    bool showPlus = false;
    bool longSpec = false;
    int64_t bw = 0;
    while(*fmt)
    {
        char c = *fmt;
        if(!specifier)
        {
            width = 0;
            hashFlag = false;
            dot = false;
            precision = 0;
            argWidth = false;
            leftJustify = false;
            zeroPad = false;
            padChr = ' ';
            showPlus = false;
            longSpec = false;
            if(c == '%')
                specifier = true;
            else
                bw += Write(&c, 1);
        }
        else
        {
            if(c == '%')
            {
                bw += Write(&c, 1);
                specifier = false;
            }
            else if(c == 'l')
                longSpec = true;
            else if(c == '.')
            {
                if(!dot)
                    dot = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c == '#')
            {
                if(!hashFlag && !dot)
                    hashFlag = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c >= '0' && c <= '9')
            {
                if(!dot)
                {
                    if(!width && c == '0' && !zeroPad)
                    {
                        padChr = '0';
                        zeroPad = true;
                    }
                    else
                    {
                        width *= 10;
                        width += c - '0';
                    }
                }
                else
                {
                    precision *= 10;
                    precision += c - '0';
                }
            }
            else if(c == '*')
            {
                if(!argWidth && !width)
                {
                    argWidth = true;
                    width = va_arg(args, int);
                }
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }

            }
            else if(c == '-')
            {
                if(!width && !dot && !leftJustify)
                    leftJustify = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c == '+')
            {
                if(!width && !dot && !showPlus)
                    showPlus = true;
                else
                {
                    bw += Write(&c, 1);
                    specifier = false;
                }
            }
            else if(c == 's' || c == 'S')
            {
                const char *str = va_arg(args, char *);
                if(!str) str = (c == 'S' ? (const char *)L"(null)" : "(null)");
                const wchar_t *STR = (wchar_t *)str;
                int len = c == 's' ? strlen(str) : wcslen(STR);
                if(dot) width = precision;
                int padc = width - len;
                padc = (padc < 0 || dot) ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                if(c == 's')
                    bw += WriteStr(str, width);
                else if(c == 'S')
                {
                    for(int i = 0; (!width || i < width) && *STR; i++)
                    {
                        bw += Write(STR, 1);
                        STR++;
                    }
                }

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'c')
            {
                char chr = va_arg(args, int);
                int padc = width - 1;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                bw += Write(&chr, 1);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'x' || c == 'X')
            {
                uint64_t val = longSpec ? va_arg(args, uint64_t) : va_arg(args, uintn);
                int maxDigits = width ? width - (hashFlag ? 2 : 0) : 16;
                int len = WriteHex(val, c == 'X', precision, maxDigits, true);
                len += hashFlag ? 2 : 0;
                int padc = width - len;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                if(hashFlag)
                    bw += WriteStr(c == 'X' ? "0X" : "0x", 0);
                bw += WriteHex(val, c == 'X', precision, maxDigits, false);

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'd' || c == 'i' || c == 'u')
            {
                int64_t val;

                if(c == 'u')
                    val = longSpec ? va_arg(args, uint64_t) :  va_arg(args, uint32_t);
                else
                    val = longSpec ? va_arg(args, int64_t) :  va_arg(args, int32_t);

                int maxDigits = width ? width : 19;
                int len = WriteDec(val, precision, maxDigits, showPlus, true, c == 'u');
                int padc = width - len;
                padc = padc < 0 ? 0 : padc;

                if(!leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                bw += WriteDec(val, precision, maxDigits, showPlus, false, c == 'u');

                if(leftJustify)
                {
                    for(int i = 0; i < padc; i++)
                        bw += Write(&padChr, 1);
                }

                specifier = false;
            }
            else if(c == 'f')
            {
                double val = va_arg(args, double);
                if(isnan(val))
                    bw += WriteStr("NaN", 0);
                else if(isinf(val))
                    bw += WriteStr(val < 0 ? "-Inf" : "+Inf", 0);
                else
                {
                    int64_t i = (int64_t)(val);
                    bw += WriteDec(i, 1, -1, showPlus, false, false);
                    bw += WriteByte('.');
                    double f = val - i;
                    if(precision)
                    {
                        precision = precision > sizeof(pow10Table) / sizeof(pow10Table[0]) ?
                                    sizeof(pow10Table) / sizeof(pow10Table[0]) - 1 : precision;
                    } else precision = 4;
                    int64_t ai = (int64_t)(f * pow10Table[precision]);
                    ai = ai < 0 ? - ai : ai;
                    bw += WriteDec(ai, precision, -1, false, false, false);
                }
                specifier = false;
            }
            else if(c == 'U') // UUID
            {
                UUID *uuid = va_arg(args, UUID *);
                if(!uuid) uuid = &UUID::nil;
                char uuidStr[40];
                uuid->ToString(uuidStr);
                if(hashFlag)
                    bw += WriteByte('{');
                bw += WriteStr(uuidStr);
                if(hashFlag)
                    bw += WriteByte('}');
                specifier = false;
            }
            else
            {
                bw += Write(&c, 1);
                specifier = false;
            }
        }
        fmt++;
    }
    return bw;
}

int64_t Stream::WriteByte(byte value)
{
    return Write(&value, 1);
}

byte Stream::ReadByte(int64_t *result)
{
    byte b;
    int64_t br = Read(&b, 1);
    if(result) *result = br;
    return b;
}

Stream::~Stream()
{
}
