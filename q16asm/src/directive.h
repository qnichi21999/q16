#ifndef DIRECTIVE_H
#define DIRECTIVE_H
#include "assembler.h"
#include "lexer.h"

struct Directive {
    const char *name;
    enum TokenKind prefix;
    void (*handle)(struct Context*, struct Token*);
};

void assemble_directive(struct Context *ctx, struct Token *tokens);
void preprocess_directive(struct Context *ctx, struct Token *tokens);

void dir_define(struct Context *ctx, struct Token *tokens);
struct Define *find_define(const char *name);


#endif
