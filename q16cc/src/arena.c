#include "arena.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

void *arena_alloc(struct Arena *arena, size_t size)
{
    assert(arena->used <= arena->capacity);
    
    printf("arena: used=%zu capacity=%zu request=%zu\n",
           arena->used,
           arena->capacity,
           size);

    if (arena->used + size > arena->capacity)
    {
        fprintf(stderr,
                "ARENA OOM: used=%zu capacity=%zu request=%zu\n",
                arena->used,
                arena->capacity,
                size);
        abort();
    }

    void *ptr = arena->data + arena->used;
    arena->used += size;
    memset(ptr, 0, size);

    return ptr;
}
void arena_reset(struct Arena *arena)
{
    arena->used = 0;
}
