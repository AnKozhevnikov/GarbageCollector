#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>

void *get_stack_base()
{
    pthread_attr_t attr;
    void *stack_addr;
    size_t stack_size;

    if (pthread_attr_init(&attr) != 0)
    {
        perror("pthread_attr_init failed");
        return NULL;
    }

    if (pthread_getattr_np(pthread_self(), &attr) != 0)
    {
        perror("pthread_attr_get_np failed");
        return NULL;
    }

    if (pthread_attr_getstack(&attr, &stack_addr, &stack_size) != 0)
    {
        perror("pthread_attr_getstack failed");
        return NULL;
    }

    pthread_attr_destroy(&attr);

    return stack_addr + stack_size;
}

void *get_stack_top()
{
    void *sp;
#if defined(__x86_64__) || defined(__amd64__)
    __asm__("movq %%rsp, %0" : "=r"(sp));
#elif defined(__i386__)
    __asm__("movl %%esp, %0" : "=r"(sp));
#elif defined(__aarch64__)
    asm("mov %0, sp" : "=r"(sp));
#elif defined(__arm__)
    asm("mov %0, sp" : "=r"(sp));
#else
    sp = NULL;
#endif
    return sp;
}

extern char __bss_start[];
extern char _end[];
extern char __data_start[];
extern char _edata[];

void *get_bss_start()
{
    return (void *)__bss_start;
}

void *get_bss_end()
{
    return (void *)_end;
}

void *get_data_start()
{
    return (void *)__data_start;
}

void *get_data_end()
{
    return (void *)_edata;
}