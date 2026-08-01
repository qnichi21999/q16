#include "preprocessor.h"
#include "assembler.h"
#include "lexer.h"
#include "directive.h"
#include <string.h>
#include <stdio.h>

struct Token tokens_pp[65536] = {0};
size_t n_tokens_pp = 0;

void preprocess_line(struct Context *ctx, struct Token *tokens, size_t len)
{
    for (int i = 0; i < len; ++i)
    {
        if (tokens[i].kind == TK_PERCENT && i == 0)
        {
            preprocess_directive(ctx, tokens);
            return;
        }
        tokens_pp[n_tokens_pp] = tokens[i];
        if (tokens[i].kind == TK_IDENT)
        {
            char name[32] = {0};
            memcpy(&name, tokens[i].start, tokens[i].len);
            struct Define *define = find_define(&name);
            if (define)
            {
                tokens_pp[n_tokens_pp] = define->value;
            }
        }
        n_tokens_pp++;
    }
}