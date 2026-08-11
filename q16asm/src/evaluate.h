#ifndef EVALUATE_H
#define EVALUATE_H

#include "lexer.h"

struct Precedence {
    enum TokenKind token_kind;
    short precedence;
};

struct Token operator_stack[64];
struct Token *op_pointer;

struct Token output_queue[64];
struct Token *out_pointer;

struct Token value_queue[32];
struct Token *value_pointer;

uint16_t evaluate(uint16_t imm, struct Token *tokens);

#endif