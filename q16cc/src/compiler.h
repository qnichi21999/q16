#ifndef COMPILER_H
#define COMPILER_H
#include "arena.h"
#include "stdint.h"
#define AST_ARENA_SIZE     (32 * 1024)
#define IR_ARENA_SIZE      (16 * 1024)
#define RODATA_ARENA_SIZE   (8 * 1024)
#define TEMP_ARENA_SIZE     (4 * 1024)
#define MEMORY_SIZE        (64 * 1024)


struct Context {
    uint8_t memory[MEMORY_SIZE];
    struct Arena ast_arena;
    struct Arena ir_arena;
    struct Arena rodata_arena;
    struct Arena temp_arena;
};


#endif