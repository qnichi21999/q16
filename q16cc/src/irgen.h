#ifndef GENERATE_IR_H
#define GENERATE_IR_H
#include "stddef.h"
#include "stdint.h"
#include "lexer.h"

struct IRGen {
    struct AstNode *ast;
    struct Arena *arena;
    size_t temporary_id;
};

enum IRKind {
    IR_PROGRAM,
    IR_FUNCTION,
    IR_INSTRUCTION,
    IR_VAR
};

enum IRInstructionKind {
    I_RETURN,
    I_UNARY
};

enum VarKind {
    V_INT,
    V_VAR
};

struct IRNode;

struct IRNodeList {
    struct IRNode **items;
    size_t count;
    size_t capacity;
};

struct Var {
    enum VarKind kind;
    union {
        const char *identifier;
        uint16_t integer;
    } value;
};

struct IRNode {
    enum IRKind kind;

    union {
        struct {
            struct IRNode *function;
        } program;

        struct {
            struct Token *name;
            struct IRNodeList instructions;
        } function;

        struct {
            enum IRInstructionKind kind;
            union {
                struct {
                    enum TokenKind op;
                    struct Var src;
                    struct Var dst;
                } i_unary;

                struct {
                    struct Var return_value;
                } i_return;
            };
        } instruction;
        
        struct {
            struct Var value;
        } val;
    };
};

struct IRNode *generate_ir(struct IRGen *irgen, struct AstNode *ast);

#endif