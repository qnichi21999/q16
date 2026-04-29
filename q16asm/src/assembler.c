#include "lexer.h"
#include "../../common/q16.h"
#include "assembler.h"
#include "string.h"
#include "strings.h"
#include "stdlib.h"
#include "stdio.h"
#include "directive.h"

static const struct Op OPS[] = {
    { "HALT",  OP_HALT,  ARG_N      },
    { "SET",   OP_SET,   ARG_R_I    },
    { "MOV",   OP_MOV,   ARG_R_R    },
    { "LOAD",  OP_LOAD,  ARG_R_BR_R },
    { "STORE", OP_STORE, ARG_R_BR_R },
    { "ADD",   OP_ADD,   ARG_R_R    },
    { "SUB",   OP_SUB,   ARG_R_R    },
    { "AND",   OP_AND,   ARG_R_R    },
    { "OR",    OP_OR,    ARG_R_R    },
    { "LSH",   OP_LSH,   ARG_R_I    },
    { "JMP",   OP_JMP,   ARG_I      },
    { "JE",    OP_JE,    ARG_R_R_I  },
    { "JG",    OP_JG,    ARG_R_R_I  },
    { "PUSH",  OP_PUSH,  ARG_R      },
    { "POP",   OP_POP,   ARG_R      },
    { "CALL",  OP_CALL,  ARG_I      },
    { "RET",   OP_RET,   ARG_N      },
    { "OUT",   OP_OUT,   ARG_R      },
};
static const size_t N_OPS = sizeof(OPS) / sizeof(OPS[0]);

void emit_byte(uint8_t *out, size_t *n, uint8_t byte)
{
    out[(*n)++] = byte;
}

void emit_type_a(uint8_t *out, size_t *n, uint8_t opcode, uint8_t reg_a, uint8_t reg_b)
{
    out[(*n)++] = opcode;
    out[(*n)++] = (reg_a << 4) | reg_b;
}

void emit_type_b(uint8_t *out, size_t *n, uint8_t opcode, uint8_t reg_a, uint8_t reg_b, uint16_t imm)
{
    out[(*n)++] = opcode;
    out[(*n)++] = (reg_a << 4) | reg_b;
    out[(*n)++] = (uint8_t)(imm >> 8);
    out[(*n)++] = (uint8_t)imm;
}

struct Op *find_op(struct Token *token)
{
    for (int i = 0; i < N_OPS; ++i)
    {
        if (token->len == strlen(OPS[i].name) &&
            strncasecmp(token->start, OPS[i].name, token->len) == 0) {
                return &OPS[i];
        }
    }
    return NULL;
}

int parse_register(struct Token *token)
{
    if(token->start[0] != 'r' && token->start[0] != 'R') return -1;
    if(token->len != 2) return -1;
    if(token->start[1] >= '0' && token->start[1] <= '7')
    {
        return token->start[1] - '0';
    }
    
    return -1; // bad register
}


struct Label *find_label(struct Context *ctx, const char *name)
{
    for (int i = 0; i < ctx->n_labels; ++i)
    {
        if (!(strncmp(ctx->labels[i].name, name, strlen(name))))
        {
            return &(ctx->labels[i]);
        }
    }
    return NULL;
}

void add_patch(struct Context *ctx, struct Token *token)
{
    memcpy(&(ctx->patches[ctx->n_patches].name), token->start, token->len);
    ctx->patches[ctx->n_patches].name[token->len] = '\0';
    ctx->patches[ctx->n_patches].offset = ctx->out_size+2;
    ctx->n_patches++;
}

void resolve_patches(struct Context *ctx)
{
    for (int i = 0; i < ctx->n_patches; ++i)
    {
        struct Label *label = find_label(ctx, ctx->patches[i].name);
        if (!label)
        {
           fprintf(stderr, "Unresolved label '%s'", ctx->patches[i].name);
           return; 
        }
        ctx->out[ctx->patches[i].offset] = (uint8_t)(label->address >> 8);
        ctx->out[ctx->patches[i].offset+1] = (uint8_t)(label->address);

    }
}

void assemble_line(struct Context *ctx, struct Token *tokens)
{
    if (tokens[0].kind == TK_DOT || tokens[0].kind == TK_PERCENT)
    {
        directive(ctx, tokens);
        return;
    } else if (tokens[0].kind != TK_IDENT) {
        return;
    }

    const struct Op *op = find_op(tokens);
    if (!op) {
        if (tokens[1].kind == TK_COLON)
        {
            char name[32];
            strncpy(name, tokens[0].start, 32);
            name[31] = 0;
            struct Label *label = find_label(ctx, name);
            if (!label)
            {
                memcpy(ctx->labels[ctx->n_labels].name, tokens[0].start, tokens[0].len);
                ctx->labels[ctx->n_labels].name[tokens[0].len] = '\0';
                ctx->labels[ctx->n_labels].address = ctx->out_size + ctx->base_address;
                ctx->n_labels++;
            }
            else
            {
                fprintf(stderr, "Label with name '%s' already exists", label->name);
            }
            
        }
        return;
    }

    switch (op->arg_type) {
        case ARG_N:
            emit_type_a(ctx->out, &(ctx->out_size), op->opcode, 0, 0);
            break;
        case ARG_R: {
            int reg_a = parse_register(&tokens[1]);
            
            if (reg_a == -1) return;
            emit_type_a(ctx->out, &(ctx->out_size), op->opcode, reg_a, 0);
            break;
        }
        case ARG_R_R: {
            int reg_a = parse_register(&tokens[1]);
            if (tokens[2].kind != TK_COMMA) {
                fprintf(stderr, "expected ','\n");
                return;
            }
            int reg_b = parse_register(&tokens[3]);
            if (reg_a == -1 || reg_b == -1) {
                fprintf(stderr, "bad register\n");
                return;
            }
            emit_type_a(ctx->out, &(ctx->out_size), op->opcode, reg_a, reg_b);
            break;
        }
        case ARG_R_I: {
            uint16_t imm = 0;
            int reg_a = parse_register(&tokens[1]);
            if (tokens[2].kind != TK_COMMA) {
                fprintf(stderr, "expected ','\n");
                return;
            }
            if (tokens[3].kind == TK_IDENT)
            {
                char name[32];
                strncpy(name, tokens[3].start, 32);
                name[31] = 0;
                struct Label *label = find_label(ctx, name);
                if (!label)
                {
                    fprintf(stderr, "Potentially unresolved label '%.*s'\n", tokens[3].len, tokens[3].start);
                    add_patch(ctx, &(tokens[3]));
                }
                else
                {
                    imm = label->address;
                }
            }
            else
            {
                if (tokens[3].kind != TK_NUMBER)
                {
                    fprintf(stderr, "expected number\n");
                    return;
                }
                else
                {
                    imm = tokens[3].value;
                }
            }

            emit_type_b(ctx->out, &(ctx->out_size), op->opcode, reg_a, 0, imm);
            break;
        }
        case ARG_I: {
            uint16_t imm = 0;
            if (tokens[1].kind == TK_IDENT)
            { 
                char name[32];
                strncpy(name, tokens[1].start, 32);
                name[31] = 0;
                struct Label *label = find_label(ctx, name);
                if (!label)
                {
                    fprintf(stderr, "Potentially unresolved label '%.*s'\n", tokens[1].len, tokens[1].start);
                    add_patch(ctx, &(tokens[1]));
                }
                else
                {
                    imm = label->address;
                }
            }
            else
            {
                if (tokens[1].kind != TK_NUMBER)
                {
                    fprintf(stderr, "expected number\n");
                    return;
                }
                else
                {
                    imm = tokens[1].value;
                }
            }
            
            
            emit_type_b(ctx->out, &(ctx->out_size), op->opcode, 0, 0, imm);
            break;
        }
        case ARG_R_R_I: {
            uint16_t imm = 0;
            int reg_a = parse_register(&tokens[1]);
            if (tokens[2].kind != TK_COMMA) {
                fprintf(stderr, "expected ','\n");
                return;
            }
            int reg_b = parse_register(&tokens[3]);
            if (tokens[4].kind != TK_COMMA) {
                fprintf(stderr, "expected ','\n");
                return;
            }
            if (reg_a == -1 || reg_b == -1) {
                fprintf(stderr, "bad register\n");
                return;
            }
            
            if (tokens[5].kind == TK_IDENT)
            {
                char name[32];
                strncpy(name, tokens[1].start, 32);
                name[31] = 0;
                struct Label *label = find_label(ctx, name);
                if (!label)
                {
                    fprintf(stderr, "Potentially unresolved label '%.*s'\n", tokens[5].len, tokens[5].start);
                    add_patch(ctx, &(tokens[5]));
                }
                else
                {
                    imm = label->address;
                }
            }
            else
            {
                if (tokens[5].kind != TK_NUMBER)
                {
                    fprintf(stderr, "expected number\n");
                    return;
                }
                else
                {
                    imm = tokens[5].value;
                }
            }
            emit_type_b(ctx->out, &(ctx->out_size), op->opcode, reg_a, reg_b, imm);
            break;
        }
        case ARG_R_BR_R: {
            int reg_a = parse_register(&tokens[1]);
            if (tokens[2].kind != TK_COMMA) {
                fprintf(stderr, "expected ','\n");
                return;
            }
            if (tokens[3].kind != TK_LBRACKET) {
                fprintf(stderr, "expected '['\n");
                return;
            }
            int reg_b = parse_register(&tokens[4]);
            if (reg_a == -1 || reg_b == -1) {
                fprintf(stderr, "bad register\n");
                return;
            }
            if (tokens[5].kind != TK_RBRACKET) {
                fprintf(stderr, "expected ']'\n");
                return;
            }
            if (op->opcode == OP_LOAD)
            {
                emit_type_a(ctx->out, &(ctx->out_size), op->opcode, reg_a, reg_b);
                break;
            }
            emit_type_a(ctx->out, &(ctx->out_size), op->opcode, reg_b, reg_a);
            break;

        }

    }
}
