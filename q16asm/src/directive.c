#include "lexer.h"
#include "assembler.h"
#include "directive.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define N_DIRECTIVES 2


void dir_org(struct Context *ctx, struct Token *address)
{
    if (address->kind != TK_NUMBER)
    {
        fprintf(stderr, "Failed to process org directive: address must be a number\n");
        return;
    }
    ctx->base_address = address->value;
    return;
}

// TODO
void dir_byte(struct Context *ctx, struct Token *tokens)
{
    if (tokens[0].kind != TK_NUMBER)
    {
        fprintf(stderr, "Failed to process byte directive: byte must be a number");
        return;
    }
}

struct Directive directives[] = {
    {"org", TK_DOT, dir_org},
    {"byte", TK_DOT, dir_byte},
};


void directive(struct Context *ctx, struct Token *tokens)
{
    if (!(tokens[0].kind == TK_PERCENT || tokens[0].kind == TK_DOT) || tokens[1].kind != TK_IDENT)
    {
        fprintf(stderr, "Failed to parse directive");
        return; 
    }
    for (int i = 0; i < N_DIRECTIVES; ++i)
    {
        if (directives[i].prefix == tokens[0].kind && !strncmp(tokens[1].start, directives[i].name, tokens[1].len))
        {
            directives[i].handle(ctx, &tokens[2]);
            return;
        }
    }
}

