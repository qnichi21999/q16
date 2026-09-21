#ifndef CODEGEN_H
#define CODEGEN_H
#include "stddef.h"
#include "stdint.h"
#include "lexer.h"
#include "irgen.h"

struct StackTable {
    const char *name;
    uint16_t offset;
};

struct CodeGen {
    struct IRNode *ir;
    struct Arena *arena;
    struct StackTable variables[1024];
    size_t n_variables;
    size_t stack_offset;
};

enum AsmKind {
    ASM_PROGRAM,
    ASM_FUNCTION,
    ASM_INSTRUCTION
};

enum AsmInstructionKind {
    ASM_I_MOV,
    ASM_I_UNARY,
    ASM_I_ALLOCATE_STACK,
    ASM_I_LOAD,
    ASM_I_STORE,
    ASM_I_SET,
    ASM_I_RET
};

struct AsmNodeList {
    struct AsmNode **items;
    size_t count;
    size_t capacity;
};

enum Register {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    SP
};

enum OperandKind {
    OP_REGISTER,
    OP_IMM,
    OP_PSEUDO,
    OP_STACK
};

struct Operand {
    enum OperandKind kind;
    union {
        enum Register reg;
        uint16_t imm;
        int stack_offset;
        const char *pseudo;
    };
};

struct AsmNode {
    enum AsmKind kind;

    union {
        struct {
            struct AsmNode *function;
        } program;

        struct {
            struct Token *name;
            struct AsmNodeList *instructions;
        } function;

        struct {
            enum AsmInstructionKind kind;
            union {
                struct {
                    struct Operand *src;
                    struct Operand *dst;
                } mov;
                struct {
                    struct Operand *reg;
                    struct Operand *address;
                    uint16_t offset;
                } load;
                struct {
                    struct Operand *reg;
                    struct Operand *address;
                    uint16_t offset;
                } store;
                struct {
                    struct Operand *reg;
                    struct Operand *imm;
                } set;
                struct {
                    enum TokenKind operator;
                    struct Operand *operand;
                } unary;
                struct {
                    size_t size;
                } allocate_stack;
            };
        } instruction;
    };
};



struct AsmNode *generate_code(struct CodeGen *c, struct IRNode *ir);

#endif