#include "evaluate.h"
#include "../../common/q16.h"
#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#define N_OPERATORS 4

const char *tok_kinds[] = {
    "TK_EOF",
    "TK_IDENT",
    "TK_NUMBER",
    "TK_COMMA",
    "TK_COLON",
    "TK_LBRACKET",
    "TK_RBRACKET",
    "TK_DOT",
    "TK_PERCENT",
    "TK_PLUS",
    "TK_MINUS",
    "TK_STAR",
    "TK_SLASH",
    "TK_LPAREN",
    "TK_RPAREN",
    "TK_EOL"
};


struct Token operator_stack[64] = {0};
struct Token *op_pointer = operator_stack;

struct Token output_queue[64] = {0};
struct Token *out_pointer = output_queue;

struct Token value_queue[32] = {0};
struct Token *value_pointer = value_queue;

struct Precedence precedence_table[] = {
    {TK_STAR, 3},
    {TK_SLASH, 3},
    {TK_PLUS, 2},
    {TK_MINUS, 2}
};

short find_precedence(struct Token *token)
{
    for (int i = 0; i < N_OPERATORS; ++i)
    {
        if(token->kind == precedence_table[i].token_kind)
        {
            return precedence_table[i].precedence;
        }
    }
    return 0;
}


uint16_t evaluate(uint16_t first_value, struct Token *tokens)
{
    op_pointer = operator_stack;
    out_pointer = output_queue;
    value_pointer = value_queue;
    struct Token *tok = tokens;
    if (tokens->kind != TK_NUMBER)
    {
        push(out_pointer, ((struct Token){TK_NUMBER, NULL, 0, first_value}));
    }
    while (tok->kind != TK_EOL && tok->kind != TK_EOF)
    {
        if (tok->kind == TK_NUMBER)
        {
            push(out_pointer, *tok);
        }
        else if (tok->kind >= TK_PLUS && tok->kind <= TK_SLASH)
        {
            while ((peek(op_pointer).kind != 0 && peek(op_pointer).kind != TK_LPAREN) && \
                    find_precedence(&(peek(op_pointer))) >= find_precedence(tok))
            {
                push(out_pointer, pop(op_pointer));
            }
            push(op_pointer, *tok);
        }
        else if (tok->kind == TK_LPAREN)
        {
            push(op_pointer, *tok);
        }
        else if (tok->kind == TK_RPAREN)
        {
            if (peek(op_pointer).kind == 0)
            {
                fprintf(stderr, "Incorrect math expression.\n");
                exit(1);
            }
            while(peek(op_pointer).kind != TK_LPAREN)
            {
                push(out_pointer, pop(op_pointer));
            }
            if (peek(op_pointer).kind != TK_LPAREN)
            {
                fprintf(stderr, "Mismatched parentheses.\n");
                exit(1);
            }
            pop(op_pointer);
        }
        tok++;
    }
    while (op_pointer > operator_stack)
    {
        if (peek(op_pointer).kind == TK_LPAREN)
        {
            fprintf(stderr, "Mismatched parentheses.\n");
            exit(1);
        }
        push(out_pointer, pop(op_pointer));
    }
    

    // for rpn output_queue needs to be read from left to right,
    // so read_pointer is a trade-off here (dont like it actually)
    struct Token *read_pointer = output_queue;

    while (read_pointer < out_pointer)
    {
        if (read_pointer->kind == TK_NUMBER)
        {
            push(value_pointer, *read_pointer);
        }
        else if (read_pointer->kind >= TK_PLUS && read_pointer->kind <= TK_SLASH)
        {
            if (peek(value_pointer).kind != TK_NUMBER)
            {
                fprintf(stderr, "Incorrect math expression (at least 2 values needed, got 0).\n");
                exit(1);
            }
            struct Token right = pop(value_pointer);
            if (peek(value_pointer).kind != TK_NUMBER)
            {
                fprintf(stderr, "Incorrect math expression (at least 2 values needed, got 1).\n");
                exit(1);
            }
            struct Token left = pop(value_pointer);
            if (read_pointer->kind == TK_PLUS)
            {
                push(value_pointer, ((struct Token){TK_NUMBER, NULL, 0, right.value + left.value}));
            }
            if (read_pointer->kind == TK_MINUS)
            {
                push(value_pointer, ((struct Token){TK_NUMBER, NULL, 0, left.value - right.value}));
            }
            if (read_pointer->kind == TK_STAR)
            {
                push(value_pointer, ((struct Token){TK_NUMBER, NULL, 0, right.value * left.value}));
            }
            if (read_pointer->kind == TK_SLASH)
            {
                push(value_pointer, ((struct Token){TK_NUMBER, NULL, 0, left.value / right.value}));
            }
        }
        read_pointer++;
    }
    return peek(value_pointer).value;
}