#include <gtest/gtest.h>

extern "C"
{
#include "gc.h"
}

TEST(GC, create_destruct)
{
    gc_create();
    gc_destruct();
}

TEST(GC, malloc_free)
{
    gc_create();
    void *ptr = gc_malloc(1);
    gc_free(ptr);
    gc_destruct();
}

TEST(GC, calloc_free)
{
    gc_create();
    void *ptr = gc_calloc(1, 1);
    gc_free(ptr);
    gc_destruct();
}

TEST(GC, realloc_free)
{
    gc_create();
    void *ptr = gc_malloc(1);
    ptr = gc_realloc(ptr, 2);
    gc_free(ptr);
    gc_destruct();
}

TEST(GC, realloc_null)
{
    gc_create();
    void *ptr = gc_realloc(NULL, 1);
    gc_free(ptr);
    gc_destruct();
}

TEST(GC, realloc_zero)
{
    gc_create();
    void *ptr = gc_malloc(1);
    ptr = gc_realloc(ptr, 0);
    gc_free(ptr);
    gc_destruct();
}

TEST(GC, realloc_null_zero)
{
    gc_create();
    void *ptr = gc_realloc(NULL, 0);
    gc_free(ptr);
    gc_destruct();
}

TEST(GC, malloc_calloc_realloc_free)
{
    gc_create();
    void *ptr1 = gc_malloc(1);
    void *ptr2 = gc_calloc(1, 1);
    ptr1 = gc_realloc(ptr1, 2);
    ptr2 = gc_realloc(ptr2, 2);
    gc_free(ptr1);
    gc_free(ptr2);
    gc_destruct();
}

TEST(GC, malloc_calloc_realloc_free_null)
{
    gc_create();
    void *ptr1 = gc_malloc(1);
    void *ptr2 = gc_calloc(1, 1);
    ptr1 = gc_realloc(ptr1, 2);
    ptr2 = gc_realloc(ptr2, 0);
    gc_free(ptr1);
    gc_free(ptr2);
    gc_destruct();
}

TEST(GC, many)
{
    gc_create();
    
    for (int i = 0; i < 100000; i++)
    {
        void *ptr = gc_malloc(1);
    }

    gc_destruct();
}

struct foo
{
    struct foo *next;
};

struct foo *dataptr = (struct foo *)5;
struct foo *bssptr;

TEST(GC, data_bss_stack)
{
    gc_create();
    struct foo *p1 = (struct foo *)gc_malloc(sizeof(struct foo));
    struct foo *p2 = (struct foo *)gc_malloc(sizeof(struct foo));
    struct foo *p3 = (struct foo *)gc_malloc(sizeof(struct foo));
    struct foo *p4 = (struct foo *)gc_malloc(sizeof(struct foo));
    struct foo *p5 = (struct foo *)gc_malloc(sizeof(struct foo));
    p1->next = p2;
    p2 = 0;
    p3 = 0;
    dataptr = p4;
    bssptr = p5;
    p4 = 0;
    p5 = 0;
    collect_garbage();
    gc_destruct();
}