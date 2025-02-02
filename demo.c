#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gc.h"

struct foo
{
    struct foo *next;
};

struct foo *dataptr = (struct foo *)5;
struct foo *bssptr;

int main(int argc, char *argv[])
{
    struct GarbageCollector gc = gc_create(__builtin_frame_address(0));
    struct foo *p1 = gc_malloc(&gc, sizeof(struct foo));
    struct foo *p2 = gc_malloc(&gc, sizeof(struct foo));
    struct foo *p3 = gc_malloc(&gc, sizeof(struct foo));
    struct foo *p4 = gc_malloc(&gc, sizeof(struct foo));
    struct foo *p5 = gc_malloc(&gc, sizeof(struct foo));
    p1->next = p2;
    p2 = 0;
    p3 = 0;
    dataptr = p4;
    bssptr = p5;
    collect_garbage(&gc);
    gc_destruct(&gc);
    return 0;
}