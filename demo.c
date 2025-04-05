#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gc.h"

struct foo
{
    struct foo *next;
    int val;
};

struct foo *dataptr = (struct foo *)5;
struct foo *bssptr;

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

    // big work
    for (int i = 0; i < 1000000000; i++)
    {
        int x = i * i;
    }

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

    // big work
    for (int i = 0; i < 1000000000; i++)
    {
        int x = i * i;
    }

    gc_unregister_thread();

    return NULL;
}

int main()
{
    gc_create();

    pthread_t thread1, thread2;

    struct foo *p1 = gc_malloc(sizeof(struct foo));
    struct foo *p2 = gc_malloc(sizeof(struct foo));
    struct foo *p3 = gc_malloc(sizeof(struct foo));

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

    assert(p1->next == p2);
    assert(p2->next == p3);
    assert(p3->next == 0);

    return 0;
}
