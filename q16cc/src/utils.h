#ifndef UTILS_H
#define UTILS_H
#include "stddef.h"

#define LIST_PUSH(arena, list, node, Type)                            \
    do {                                                              \
        if ((list)->count == (list)->capacity) {                    \
            size_t new_capacity =                                    \
                (list)->capacity == 0 ? 4 : (list)->capacity * 2;   \
            Type **new_items = arena_alloc(                          \
                (arena), new_capacity * sizeof(*new_items)          \
            );                                                        \
            for (size_t i = 0; i < (list)->count; ++i)               \
                new_items[i] = (list)->items[i];                    \
            (list)->items = new_items;                              \
            (list)->capacity = new_capacity;                        \
        }                                                             \
        (list)->items[(list)->count++] = (node);                     \
    } while (0)

void print_tokens(struct Token *tokens, size_t token_count);
const char *token_kind_name(enum TokenKind kind);
void print_ast(struct AstNode *node, size_t tab_count);
void print_ir(struct IRNode *node, size_t tab_count);
void print_asm_ir(struct AsmNode *node, size_t tab_count);

#endif