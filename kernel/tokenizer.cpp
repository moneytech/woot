#include <stdlib.h>
#include <string.h>
#include <tokenizer.h>

Tokenizer::Tokenizer(const char *string, const char *delims, size_t maxTokens) :
    stringCopy(strdup(string))
{
    char *nextp = nullptr;
    for(int i = 0; ; ++i)
    {
        bool tail = maxTokens && i >= (maxTokens - 1);
        char *s = tail ? nextp : strtok_r(i ? nullptr : stringCopy, delims, &nextp);
        if(!s) break;

        // trim any possible leading delimiters in tail
        if(tail)
        {
            for(; *s; ++s)
            {
                bool done = true;
                for(int j = 0; delims[j]; ++j)
                {
                    if(*s == delims[j])
                    {
                        done = false;
                        break;
                    }
                }
                if(done) break;
            }
        }

        Tokens.Append(Token(s, s - stringCopy));
        if(tail) break;
    }
}

char *Tokenizer::operator[](uint idx)
{
    return Tokens[idx].String;
}

Tokenizer::~Tokenizer()
{
    free(stringCopy);
}
