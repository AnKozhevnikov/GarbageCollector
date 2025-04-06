#ifndef GC_H
#define GC_H

#ifdef __cplusplus
    #include <atomic>
    typedef std::atomic<int> atomic_int;
#else
    #include <stdatomic.h>
#endif
#include <pthread.h>
#include "hashmap.h"
#include "memory_access.h"

unsigned hash_for_pointer(const void *value);
unsigned hash_for_thread(const void *value);

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
    struct HashMap *allocations;
    struct HashMap *threads;
    unsigned paused;

    atomic_int threads_to_scan;
    atomic_int allocation_cnt;
    atomic_int threads_registring;

    unsigned allocation_threshold;

    pthread_mutex_t collect_garbage_mutex;
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

void gc_pause();
void gc_resume();

int get_alive_allocations();

#endif // GC_H