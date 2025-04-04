#ifndef MEMORY_ACCESS_H
#define MEMORY_ACCESS_H

#include <stdio.h>

void *get_stack_base();
void *get_stack_top();

void *get_bss_start();
void *get_bss_end();
void *get_data_start();
void *get_data_end();

#endif // MEMORY_ACCESS_H