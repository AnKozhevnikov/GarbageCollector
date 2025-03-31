#ifndef MEMORY_ACCESS_H
#define MEMORY_ACCESS_H

#include <stdio.h>

extern char __bss_start[];
extern char _end[];
extern char __data_start[];
extern char _edata[];

void *get_stack_base();
void *get_stack_top();

#endif // MEMORY_ACCESS_H