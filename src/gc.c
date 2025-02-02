#include "gc.h"
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>

extern unsigned hash_for_pointer(const void *value)
{
    return (unsigned)(uintptr_t)value;
}

void gc_activate(struct GarbageCollector *gc, void *ptr)
{
    struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
    alloc->active = 1;
}

void gc_deactivate(struct GarbageCollector *gc, void *ptr)
{
    struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
    alloc->active = 0;
}

struct GarbageCollector gc_create(void *bottom)
{
    struct GarbageCollector gc;
    gc.allocations = hashmap_create(sizeof(void *), sizeof(void *), hash_for_pointer);
    gc.paused = 0;
    gc.bottom = bottom;
    return gc;
}

void gc_destruct(struct GarbageCollector *gc)
{
    for (unsigned i = 0; i < gc->allocations.capacity; i++)
    {
        if (gc->allocations.used[i])
        {
            struct Allocation *alloc =
                *(struct Allocation **)(gc->allocations.values +
                                      i * (gc->allocations.value_size + gc->allocations.key_size) +
                                      gc->allocations.key_size);
            free(alloc->ptr);
            free(alloc);
        }
    }
    hashmap_destruct(&gc->allocations);
}

void *gc_malloc(struct GarbageCollector *gc, size_t size)
{
    struct Allocation *alloc = malloc(sizeof(struct Allocation));
    alloc->ptr = malloc(size);
    alloc->size = size;
    alloc->active = 1;
    hashmap_insert(&gc->allocations, &alloc->ptr, &alloc);
    return alloc->ptr;
}

void *gc_calloc(struct GarbageCollector *gc, size_t nmemb, size_t size)
{
    struct Allocation *alloc = malloc(sizeof(struct Allocation));
    alloc->ptr = calloc(nmemb, size);
    alloc->size = nmemb * size;
    alloc->active = 1;
    hashmap_insert(&gc->allocations, &alloc->ptr, &alloc);
    return alloc->ptr;
}

void *gc_realloc(struct GarbageCollector *gc, void *ptr, size_t size)
{
    struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
    hashmap_erase(&gc->allocations, &ptr);
    alloc->ptr = realloc(alloc->ptr, size);
    alloc->size = size;
    hashmap_insert(&gc->allocations, &alloc->ptr, &alloc);
    return alloc->ptr;
}

void gc_free(struct GarbageCollector *gc, void *ptr)
{
    struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
    hashmap_erase(&gc->allocations, &ptr);
    free(alloc->ptr);
    free(alloc);
}

void gc_dfs(struct GarbageCollector *gc, struct Allocation *alloc)
{
    if (alloc->used)
    {
        return;
    }
    alloc->used = 1;
    void *start = alloc->ptr;
    void *end = start + alloc->size;
    for (void *i = start; i < end; i += 8)
    {
        void *ptr = *(void **)i;
        if (hashmap_contains(&gc->allocations, &ptr))
        {
            struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
            if (alloc->active && !alloc->root)
            {
                gc_dfs(gc, alloc);
            }
        }
    }
}

void mark(struct GarbageCollector *gc)
{
    ucontext_t context;
    getcontext(&context);
    for (unsigned i = 0; i < gc->allocations.capacity; i++)
    {
        if (gc->allocations.used[i / 8] & (1 << (i % 8)))
        {
            struct Allocation *alloc =
                *(struct Allocation **)(gc->allocations.values +
                                      i * (gc->allocations.value_size + gc->allocations.key_size) +
                                      gc->allocations.key_size);
            alloc->root = 0;
            alloc->used = 0;
        }
    }

    void *top = __builtin_frame_address(0);

    for (void *i = top; i < gc->bottom; i += 8)
    {
        void *ptr = *(void **)i;
        if (hashmap_contains(&gc->allocations, &ptr))
        {
            struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
            if (alloc->active)
            {
                alloc->root = 1;
                gc_dfs(gc, alloc);
            }
        }
    }

    top = (void*)__data_start;
    for (void *i = top; i < (void*)_edata; i += 8)
    {
        void *ptr = *(void **)i;
        if (hashmap_contains(&gc->allocations, &ptr))
        {
            struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
            if (alloc->active)
            {
                alloc->root = 1;
                gc_dfs(gc, alloc);
            }
        }
    }

    top = (void*)__bss_start;
    for (void *i = top; i < (void*)_end; i += 8)
    {
        void *ptr = *(void **)i;
        if (hashmap_contains(&gc->allocations, &ptr))
        {
            struct Allocation *alloc = *(struct Allocation **)hashmap_find(&gc->allocations, &ptr);
            if (alloc->active)
            {
                alloc->root = 1;
                gc_dfs(gc, alloc);
            }
        }
    }
}

void sweep(struct GarbageCollector *gc)
{
    for (unsigned i = 0; i < gc->allocations.capacity; i++)
    {
        if (gc->allocations.used[i])
        {
            struct Allocation *alloc =
                *(struct Allocation **)(gc->allocations.values +
                                      i * (gc->allocations.value_size + gc->allocations.key_size) +
                                      gc->allocations.key_size);
            if (!alloc->used && alloc->active)
            {
                hashmap_erase(&gc->allocations, &alloc->ptr);
                free(alloc->ptr);
                free(alloc);
            }
        }
    }
}

void collect_garbage(struct GarbageCollector *gc)
{
    mark(gc);
    sweep(gc);
}
