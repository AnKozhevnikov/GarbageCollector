#ifndef GC_H
#define GC_H

#include "hashmap.h"
#include "memory_access.h"
#include <pthread.h>
#include <stdatomic.h>

unsigned hash_for_pointer(const void *value);
unsigned hash_for_thread(pthread_t value);

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
    struct HashMap threads;
    unsigned paused;
    void *bottom;

    atomic_int threads_to_scan;
    atomic_int allocation_cnt;

    unsigned allocation_threshold;
};

void gc_activate(void *ptr);
void gc_deactivate(void *ptr);

void gc_create();
void gc_destruct();
void* gc_malloc(size_t size);
void gc_free(void *ptr);
void* gc_calloc(size_t nmemb, size_t size);
void* gc_realloc(void *ptr, size_t size);

void gc_register_thread();
void gc_unregister_thread();

void mark_stack();
void mark_sections();
void sweep();

void collect_garbage();

#endif // GC_H