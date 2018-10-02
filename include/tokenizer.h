#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <list.h>
#include <types.h>

class Tokenizer
{
    char *stringCopy;
public:
    class Token
    {
    public:
        char *String;
        int Offset;

        Token() : String(nullptr), Offset(-1) {}
        Token(char *string, int offset) : String(string), Offset(offset) {}
    };

    List<Token> Tokens;

    Tokenizer(const char *string, const char *delims, size_t maxTokens);
    char *operator[](uint idx);
    ~Tokenizer();
};

#endif // TOKENIZER_H
