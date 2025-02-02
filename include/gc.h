#pragma once

#include "hashmap.h"

extern char __bss_start[];
extern char _end[];
extern char __data_start[];
extern char _edata[];

unsigned hash_for_pointer(const void *value);

struct Allocation
{
    void *ptr;
    size_t size;
    char active;
    char root;
    char used;
};

struct GarbageCollector
{
    struct HashMap allocations;
    unsigned paused;
    void *bottom;
};

void gc_activate(struct GarbageCollector *gc, void *ptr);
void gc_deactivate(struct GarbageCollector *gc, void *ptr);

struct GarbageCollector gc_create(void *bottom);
void gc_destruct(struct GarbageCollector *gc);
void* gc_malloc(struct GarbageCollector *gc, size_t size);
void gc_free(struct GarbageCollector *gc, void *ptr);
void* gc_calloc(struct GarbageCollector *gc, size_t nmemb, size_t size);
void* gc_realloc(struct GarbageCollector *gc, void *ptr, size_t size);

void mark(struct GarbageCollector *gc);
void sweep(struct GarbageCollector *gc);

void collect_garbage(struct GarbageCollector *gc);