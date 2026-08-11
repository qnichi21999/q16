#include "lexer.h"
#include "assembler.h"
#include "directive.h"
#include "evaluate.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define N_ASSEMBLE_DIRECTIVES 2
#define N_PREPROCESS_DIRECTIVES 1

struct DefineTable define_table = {0};

struct Define *find_define(const char *name)
{
    for (int i = 0; i < define_table.n_defines; ++i)
    {
       
        if (!(strcmp(define_table.defines[i].name, name)))
        {
            return &(define_table.defines[i]);
        }
    }
    return NULL;
}

void dir_define(struct Context *ctx, struct Token *tokens)
{
    if (tokens[0].kind != TK_IDENT)
    {
        fprintf(stderr, "Failed to process define directive: first argument must be an identifier\n");
        return;
    }
    char name[32];
    strncpy(&name, tokens[0].start, tokens[0].len);
    struct Define *define = find_define(&name);
    
    if (!define)
    {
        strncpy(&(define_table.defines[define_table.n_defines].name), tokens[0].start, tokens[0].len);
        define_table.defines[define_table.n_defines].name[tokens[0].len] = '\0';
        define_table.defines[define_table.n_defines].value = tokens[1];
        define_table.n_defines++;
        return;
    }
    fprintf(stderr, "Define '%s' already exists\n", define->name);
    return;
}

void dir_org(struct Context *ctx, struct Token *tokens)
{
    if (tokens->kind != TK_NUMBER)
    {
        fprintf(stderr, "Failed to process org directive: address must be a number\n");
        return;
    }
    ctx->base_address = evaluate(tokens[0].value, tokens);
    return;
}

char process_char(struct Token *tokens)
{
    if (*(tokens[0].start) > 255)
    {
        fprintf(stderr, "Failed to process .byte directive: given character must be in ASCII range\n");
        exit(1);
    }
    if (tokens[1].kind != TK_QUOTE)
    {
        fprintf(stderr, "Failed to process .byte directive: missing closing \'\n");
        exit(1);
    }
    return *(tokens[0].start);
}

void dir_byte(struct Context *ctx, struct Token *tokens)
{
    if (tokens[0].kind != TK_NUMBER && tokens[0].kind != TK_QUOTE)
    {
        fprintf(stderr, "Failed to process .byte directive: byte must be a number or a character\n");
        exit(1);
    }
    int n = 0;
    struct Token argument[32] = {0};
    struct Token *tok = tokens;
    while (1)
    {
        if (tok->kind == TK_COMMA || tok->kind == TK_EOL || tok->kind == TK_EOF)
        {
            argument[n] = (struct Token){TK_EOL, NULL, 0, 0};
            if (argument[0].kind == TK_NUMBER || argument[0].kind == TK_LPAREN)
            {
                uint16_t value = evaluate(argument[0].value, argument);
                if (value > 255)
                {
                    fprintf(stderr, "Failed to process .byte directive: given value must not exceed 255\n");
                    exit(1);
                }
                emit_byte(&(ctx->out), &(ctx->out_size), (uint8_t)value);
            }
            else if (argument[0].kind == TK_QUOTE)
            {
                emit_byte(&(ctx->out), &(ctx->out_size), (uint8_t)process_char(&(argument[1])));
            }
            if (tok->kind == TK_EOL || tok->kind == TK_EOF) return;

            memset(argument, 0, sizeof(argument));
            n = 0;
            tok++;
        }
        else
        {
           argument[n] = *tok;
           n++;
           tok++;
        }
    }
    return;
}

struct Directive assemble_directives[] = {
    {"org", TK_DOT, dir_org},
    {"byte", TK_DOT, dir_byte},
};

struct Directive preprocess_directives[] = {
    {"define", TK_PERCENT, dir_define}
};


void assemble_directive(struct Context *ctx, struct Token *tokens)
{
    if (!(tokens[0].kind == TK_DOT) || tokens[1].kind != TK_IDENT)
    {

        fprintf(stderr, "Failed to parse directive (assemble)\n");
        return; 
    }
    if (tokens[0].kind == TK_DOT)
    {
        for (int i = 0; i < N_ASSEMBLE_DIRECTIVES; ++i)
        {
            if (assemble_directives[i].prefix == tokens[0].kind && !strncmp(tokens[1].start, assemble_directives[i].name, tokens[1].len))
            {
                assemble_directives[i].handle(ctx, &tokens[2]);
                return;
            }
        }
    }
}

void preprocess_directive(struct Context *ctx, struct Token *tokens)
{
    if (!(tokens[0].kind == TK_PERCENT) || tokens[1].kind != TK_IDENT)
    {
        fprintf(stderr, "Failed to parse directive (preprocess)\n");
        return; 
    }
    if (tokens[0].kind == TK_PERCENT)
    {
        for (int i = 0; i < N_PREPROCESS_DIRECTIVES; ++i)
        {
            if (preprocess_directives[i].prefix == tokens[0].kind && !strncmp(tokens[1].start, preprocess_directives[i].name, tokens[1].len))
            {
                preprocess_directives[i].handle(ctx, &tokens[2]);
                return;
            }
        }
    }
    
}

