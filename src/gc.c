#include "gc.h"
#include "global.h"
#include "sched.h"
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

unsigned hash_for_pointer(const void *value)
{
    return (unsigned)*(uintptr_t *)value;
}

unsigned hash_for_thread(const void *value)
{
    return (unsigned)(pthread_t *)value;
}

void gc_activate(void *ptr)
{
    struct Iterator it = hashmap_find(&gc->allocations, &ptr);
    struct Allocation *alloc = *(struct Allocation **)it.value;
    alloc->active = 1;
}

void gc_deactivate(void *ptr)
{
    struct Iterator it = hashmap_find(&gc->allocations, &ptr);
    struct Allocation *alloc = *(struct Allocation **)it.value;
    alloc->active = 0;
}

void gc_create()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    gc = malloc(sizeof(struct GarbageCollector));
    gc->allocations = hashmap_create(sizeof(void *), sizeof(void *), hash_for_pointer);
    gc->threads = hashmap_create(sizeof(pthread_t), 0, hash_for_thread);
    gc->paused = 0;
    gc->allocation_threshold = 1000;
    gc->threads_to_scan = 0;
    gc->allocation_cnt = 0;
    gc->threads_registring = 0;
    gc->collect_garbage_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    gc_register_thread();
}

void gc_destruct()
{
    struct Iterator it = hashmap_begin(&gc->allocations);
    while (hashmap_not_end(it))
    {
        struct Allocation *alloc = *(struct Allocation **)it.value;
        free(alloc->ptr);
        free(alloc);
        it = hashmap_next(it);
    }
    hashmap_destruct(&gc->allocations);
    hashmap_destruct(&gc->threads);

    pthread_mutex_destroy(&gc->collect_garbage_mutex);
}

void *gc_malloc(size_t size)
{
    struct Allocation *alloc = malloc(sizeof(struct Allocation));
    alloc->ptr = malloc(size);
    alloc->size = size;
    alloc->active = 1;
    hashmap_insert(&gc->allocations, &alloc->ptr, &alloc);

    if (atomic_fetch_add(&gc->allocation_cnt, 1) > gc->allocation_threshold && !gc->paused)
    {
        collect_garbage();
        atomic_store(&gc->allocation_cnt, 0);
    }

    return alloc->ptr;
}

void *gc_calloc(size_t nmemb, size_t size)
{
    struct Allocation *alloc = malloc(sizeof(struct Allocation));
    alloc->ptr = calloc(nmemb, size);
    alloc->size = nmemb * size;
    alloc->active = 1;
    hashmap_insert(&gc->allocations, &alloc->ptr, &alloc);

    if (atomic_fetch_add(&gc->allocation_cnt, 1) > gc->allocation_threshold && !gc->paused)
    {
        collect_garbage();
        atomic_store(&gc->allocation_cnt, 0);
    }

    return alloc->ptr;
}

void *gc_realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
    {
        return gc_malloc(size);
    }
    struct Iterator it = hashmap_find(&gc->allocations, &ptr);
    struct Allocation *alloc = *(struct Allocation **)it.value;
    hashmap_erase(&gc->allocations, &ptr);
    alloc->ptr = realloc(alloc->ptr, size);
    alloc->size = size;
    hashmap_insert(&gc->allocations, &alloc->ptr, &alloc);

    if (atomic_fetch_add(&gc->allocation_cnt, 1) > gc->allocation_threshold && !gc->paused)
    {
        collect_garbage();
        atomic_store(&gc->allocation_cnt, 0);
    }

    return alloc->ptr;
}

void gc_free(void *ptr)
{
    struct Iterator it = hashmap_find(&gc->allocations, &ptr);
    struct Allocation *alloc = *(struct Allocation **)it.value;
    hashmap_erase(&gc->allocations, &ptr);
    free(alloc->ptr);
    free(alloc);
}

void gc_dfs(struct Allocation *alloc)
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
            struct Iterator it = hashmap_find(&gc->allocations, &ptr);
            struct Allocation *alloc = *(struct Allocation **)it.value;
            if (alloc->active && !alloc->root)
            {
                gc_dfs(alloc);
            }
        }
    }
}

pthread_mutex_t gc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gc_cond = PTHREAD_COND_INITIALIZER;

void mark_stack()
{
    jmp_buf buf;
    int ret = setjmp(buf);

    void *top = get_stack_top();
    void *bottom = get_stack_base();
    if (top < bottom)
        for (void *i = top; i < bottom; i += 8)
        {
            void *ptr = *(void **)i;
            if (hashmap_contains(&gc->allocations, &ptr))
            {
                struct Iterator it = hashmap_find(&gc->allocations, &ptr);
                struct Allocation *alloc = *(struct Allocation **)it.value;
                if (alloc->active)
                {
                    alloc->root = 1;
                    gc_dfs(alloc);
                }
            }
        }
    else
        for (void *i = bottom; i < top; i += 8)
        {
            void *ptr = *(void **)i;
            if (hashmap_contains(&gc->allocations, &ptr))
            {
                struct Iterator it = hashmap_find(&gc->allocations, &ptr);
                struct Allocation *alloc = *(struct Allocation **)it.value;
                if (alloc->active)
                {
                    alloc->root = 1;
                    gc_dfs(alloc);
                }
            }
        }

    atomic_fetch_sub(&gc->threads_to_scan, 1);

    pthread_mutex_lock(&gc_mutex);
    pthread_cond_signal(&gc_cond);
    pthread_mutex_unlock(&gc_mutex);
}

void mark_sections()
{
    void *top = get_data_start();
    void *bottom = get_data_end();
    for (void *i = top; i < bottom; i += 8)
    {
        void *ptr = *(void **)i;
        if (hashmap_contains(&gc->allocations, &ptr))
        {
            struct Iterator it = hashmap_find(&gc->allocations, &ptr);
            struct Allocation *alloc = *(struct Allocation **)it.value;
            if (alloc->active)
            {
                alloc->root = 1;
                gc_dfs(alloc);
            }
        }
    }

    top = get_bss_start();
    bottom = get_bss_end();
    for (void *i = top; i < bottom; i += 8)
    {
        void *ptr = *(void **)i;
        if (hashmap_contains(&gc->allocations, &ptr))
        {
            struct Iterator it = hashmap_find(&gc->allocations, &ptr);
            struct Allocation *alloc = *(struct Allocation **)it.value;
            if (alloc->active)
            {
                alloc->root = 1;
                gc_dfs(alloc);
            }
        }
    }
}

void sweep()
{
    struct Iterator it = hashmap_begin(&gc->allocations);
    while (hashmap_not_end(it))
    {
        struct Allocation *alloc = *(struct Allocation **)it.value;
        if (!alloc->used && alloc->active)
        {
            hashmap_erase(&gc->allocations, &alloc->ptr);
            free(alloc->ptr);
            free(alloc);
        }
        it = hashmap_next(it);
    }
}

void collect_garbage()
{
    pthread_mutex_lock(&gc->collect_garbage_mutex);

    struct Iterator it = hashmap_begin(&gc->allocations);
    while (hashmap_not_end(it))
    {
        struct Allocation *alloc = *(struct Allocation **)it.value;
        alloc->root = 0;
        alloc->used = 0;
        it = hashmap_next(it);
    }

    mark_sections();

    pthread_t self = pthread_self();
    atomic_store(&gc->threads_to_scan, gc->threads.size);
    if (hashmap_contains(&gc->threads, &self))
    {
        mark_stack();
    }

    pthread_mutex_lock(&gc_mutex);

    while (atomic_load(&gc->threads_registring) != 0)
    {
        sched_yield();
    }
    it = hashmap_begin(&gc->threads);
    while (hashmap_not_end(it))
    {
        pthread_t thread = *(pthread_t *)it.key;
        if (thread != self)
        {
            if (pthread_kill(thread, SIGUSR1) != 0)
            {
                perror("pthread_kill failed");
                atomic_fetch_sub(&gc->threads_to_scan, 1);
            }
        }
        it = hashmap_next(it);
    }

    while (atomic_load(&gc->threads_to_scan))
    {
        pthread_cond_wait(&gc_cond, &gc_mutex);
    }

    pthread_mutex_unlock(&gc_mutex);

    sweep();

    pthread_mutex_unlock(&gc->collect_garbage_mutex);
}

void gc_signal_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        mark_stack();
    }
}

void gc_register_thread()
{
    atomic_fetch_add(&gc->threads_registring, 1);

    pthread_t self = pthread_self();
    if (hashmap_contains(&gc->threads, &self))
    {
        return;
    }
    hashmap_insert(&gc->threads, &self, 0);

    struct sigaction sa;
    sa.sa_handler = &gc_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction failed");
    }
    atomic_fetch_sub(&gc->threads_registring, 1);
}

void gc_unregister_thread()
{
    pthread_t self = pthread_self();
    if (!hashmap_contains(&gc->threads, &self))
    {
        return;
    }
    hashmap_erase(&gc->threads, &self);
}

void gc_pause()
{
    gc->paused = 1;
}

void gc_resume()
{
    gc->paused = 0;
}

int get_alive_allocations()
{
    return gc->allocations.size;
}