#include "lexer.h"
#include "assembler.h"
#include "directive.h"
#include "evaluate.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
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

void dir_byte(struct Context *ctx, struct Token *tokens)
{
    if (tokens[0].kind != TK_NUMBER)
    {
        fprintf(stderr, "Failed to process byte directive: byte must be a number\n");
        return;
    }
    if (tokens[0].value > 255)
    {
        fprintf(stderr, "Failed to process byte directive: given value must not exceed 255\n");
        return;
    }
    ctx->out[ctx->out_size++] = evaluate(tokens[0].value, tokens);
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

