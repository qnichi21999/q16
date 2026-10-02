#ifndef EMIT_H
#define EMIT_H
#include <stdint.h>

void emit_asm(char *output_path, struct AsmNode *node);

enum HelperFlags {
    HELPER_DIV = 1 << 0,
    HELPER_MOD = 1 << 1,
    HELPER_MUL = 1 << 2
};

extern uint8_t helper_flags;

#endif