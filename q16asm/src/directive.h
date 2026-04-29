#ifndef DIRECTIVE_H
#define DIRECTIVE_H
#include "assembler.h"
#include "lexer.h"

struct Directive {
    const char *name;
    enum TokenKind prefix;
    void (*handle)(struct Context*, struct Token*);
};

void directive(struct Context *ctx, struct Token *tokens);

#endif
