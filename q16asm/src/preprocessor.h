#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H
#include "assembler.h"
#include "lexer.h"

extern struct Token tokens_pp[65536];
extern size_t n_tokens_pp;

void preprocess_line(struct Context *ctx, struct Token *tokens, size_t len);

#endif