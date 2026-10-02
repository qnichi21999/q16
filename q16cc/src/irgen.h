#ifndef GENERATE_IR_H
#define GENERATE_IR_H
#include "stddef.h"
#include "stdint.h"
#include "lexer.h"

struct IRGen {
    struct AstNode *ast;
    struct Arena *arena;
    size_t temporary_id;
    size_t label_id;
};

enum IRKind {
    IR_PROGRAM,
    IR_FUNCTION,
    IR_INSTRUCTION,
    IR_VAR
};

enum IRInstructionKind {
    I_RETURN,
    I_UNARY,
    I_BINARY,
    I_COPY,
    I_JMP,
    I_JZ,
    I_JNZ,
    I_JG,
    I_JE,
    I_LABEL
};

enum VarKind {
    V_INT,
    V_VAR
};

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
                    enum TokenKind op;
                    struct Var src1;
                    struct Var src2;
                    struct Var dst;
                } i_binary;

                struct {
                    struct Var return_value;
                } i_return;

                struct {
                    struct Var src;
                    struct Var dst;
                } i_copy;

                struct {
                    struct IRNode *target;
                } i_jmp;

                struct {
                    struct Var src;
                    struct IRNode *target;
                } i_jz;

                struct {
                    struct Var src;
                    struct Var dst;
                    struct IRNode *target;
                } i_je;

                struct {
                    struct Var src;
                    struct Var dst;
                    struct IRNode *target;
                } i_jg;

                struct {
                    char *name;
                } i_label;
            };
        } instruction;
        
        struct {
            struct Var value;
        } val;
    };
};

struct IRNode *generate_ir(struct IRGen *irgen, struct AstNode *ast);

#endif