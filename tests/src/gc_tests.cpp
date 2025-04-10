#include <gtest/gtest.h>

extern "C"
{
#include "gc.h"
}

TEST(GC, create_destruct)
{
    gc_create();

    printf("Created garbage collector\n");

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
    int val;
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

volatile int assigned1 = 0;
volatile int assigned2 = 0;
volatile int ready1 = 0;
volatile int ready2 = 0;

void *func1(void *arg)
{
    gc_register_thread();

    struct foo **arr = (struct foo **)arg;
    struct foo *p1 = arr[0];
    struct foo *p2 = arr[1];
    p1->next = p2;
    assigned1 = 1;

    ready1 = 1;
    printf("func1 ready and waiting for signal\n");

    pause();

    gc_unregister_thread();

    return NULL;
}

void *func2(void *arg)
{
    gc_register_thread();

    struct foo **arr = (struct foo **)arg;
    struct foo *p2 = arr[1];
    struct foo *p3 = arr[2];
    p2->next = p3;
    p3->next = 0;
    assigned2 = 1;

    ready2 = 1;
    printf("func2 ready and waiting for signal\n");

    pause();

    gc_unregister_thread();

    return NULL;
}


TEST(GC, multi_thread)
{
    gc_create();

    pthread_t thread1, thread2;

    struct foo *p1 = (struct foo *)gc_malloc(sizeof(struct foo));
    struct foo *p2 = (struct foo *)gc_malloc(sizeof(struct foo));
    struct foo *p3 = (struct foo *)gc_malloc(sizeof(struct foo));

    p1->val = 1;
    p2->val = 2;
    p3->val = 3;

    struct foo *arr[3];
    arr[0] = p1;
    arr[1] = p2;
    arr[2] = p3;

    pthread_create(&thread1, NULL, func1, arr);
    pthread_create(&thread2, NULL, func2, arr);

    while (!ready1 || !ready2)
        usleep(1000);

    gc_unregister_thread();

    arr[0] = 0;
    arr[1] = 0;
    arr[2] = 0;

    printf("Main sending SIGUSR1 to threads\n");
    collect_garbage();

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    ASSERT_EQ(p1->next, p2);
    ASSERT_EQ(p2->next, p3);
    ASSERT_EQ(p3->next, (struct foo *)0);
}
