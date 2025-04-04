#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <pthread.h>
#include <stdio.h>

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
    __asm__ __volatile__("movq %%rsp, %0" : "=r"(sp));
#elif defined(__i386__)
    __asm__ __volatile__("movl %%esp, %0" : "=r"(sp));
#elif defined(__aarch64__)
    __asm__ __volatile__("mov %0, sp" : "=r"(sp));
#elif defined(__arm__)
    __asm__ __volatile__("mov %0, sp" : "=r"(sp));
#else
    sp = NULL;
#endif
    return sp;
}

void *get_bss_start()
{
    uint64_t bss_size;
    const struct mach_header_64 *header = (struct mach_header_64 *)_dyld_get_image_header(0);
    uint8_t *bss_start = (uint8_t *)getsectiondata(header, "__DATA", "__bss", &bss_size);
    return (void *)bss_start;
}

void *get_bss_end()
{
    uint64_t bss_size;
    const struct mach_header_64 *header = (struct mach_header_64 *)_dyld_get_image_header(0);
    uint8_t *bss_start = (uint8_t *)getsectiondata(header, "__DATA", "__bss", &bss_size);
    return (void *)(bss_start + bss_size);
}

void *get_data_start()
{
    uint64_t data_size;
    const struct mach_header_64 *header = (struct mach_header_64 *)_dyld_get_image_header(0);
    uint8_t *data_start = (uint8_t *)getsectiondata(header, "__DATA", "__data", &data_size);
    return (void *)data_start;
}

void *get_data_end()
{
    uint64_t data_size;
    const struct mach_header_64 *header = (struct mach_header_64 *)_dyld_get_image_header(0);
    uint8_t *data_start = (uint8_t *)getsectiondata(header, "__DATA", "__data", &data_size);
    return (void *)(data_start + data_size);
}