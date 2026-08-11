#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include "../../common/q16.h"
#include "lexer.h"
#include <stdint.h>
#include <stdbool.h>

enum Arg {
    ARG_N,         // HALT, RET
    ARG_R,         // PUSH, POP, OUT
    ARG_I,         // JMP, CALL
    ARG_R_R,       // MOV, ADD, SUB, AND, OR
    ARG_R_I,       // SET, LSH
    ARG_R_R_I,     // JE, JG
    ARG_R_BR_R,    // LOAD, STORE
};

struct Op {
    const char name[8];
    OpCode opcode;
    enum Arg arg_type;
};

struct Label {
    char name[32];
    uint16_t address;
};

struct Patch {
    char name[32];
    struct Token line[32];
    uint16_t offset;
    bool resolved;
};

struct Define {
    char name[32];
    struct Token value;
};

struct DefineTable {
    struct Define defines[256];
    uint8_t n_defines;
};

struct Context {
    uint8_t out[65536];
    size_t out_size;
    struct Label labels[256];
    uint8_t n_labels;
    struct Patch patches[256];
    uint8_t n_patches;
    uint16_t base_address;
};

void assemble_line(struct Context *ctx, struct Token *tokens);
void resolve_patches(struct Context *ctx);
void emit_byte(uint8_t *out, size_t *n, uint8_t byte);

#endif
