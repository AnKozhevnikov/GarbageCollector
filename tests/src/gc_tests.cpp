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