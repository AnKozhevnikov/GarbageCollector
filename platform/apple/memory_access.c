#include <pthread.h>

void *get_stack_base()
{
    pthread_t thread = pthread_self();
    void *stack_addr = pthread_get_stackaddr_np(thread);
    size_t stack_size = pthread_get_stacksize_np(thread);

    return (void *)((char *)stack_addr - stack_size);
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
