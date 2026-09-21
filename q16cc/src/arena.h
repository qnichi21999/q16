#ifndef ARENA_H
#define ARENA_H
#include "stddef.h"


struct Arena {
    char *data;
    size_t capacity;
    size_t used;
};

void *arena_alloc(struct Arena *arena, size_t size);
void arena_reset(struct Arena *arena);

#endif