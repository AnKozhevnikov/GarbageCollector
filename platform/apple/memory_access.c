#include <mach-o/getsect.h>
#include <mach-o/dyld.h>
#include <pthread.h>
#include <stdio.h>
#include <libgen.h> 
#include <unistd.h> 
#include <string.h>
#include <limits.h>
#include <stdlib.h>

const struct mach_header_64 *get_own_image_header() {
    char exe_path[PATH_MAX] = {0};
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) != 0) {
        fprintf(stderr, "Buffer too small for executable path\n");
        return NULL;
    }

    char real_path[PATH_MAX];
    realpath(exe_path, real_path);

    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char *image_name = _dyld_get_image_name(i);
        if (image_name == NULL) continue;

        char image_real[PATH_MAX];
        if (realpath(image_name, image_real) == NULL) continue;

        if (strcmp(real_path, image_real) == 0) {
            const struct mach_header *header = (const struct mach_header_64 *)_dyld_get_image_header(i);
            if (header->magic == MH_MAGIC_64) {
                return (const struct mach_header_64 *)header;
            }
        }
    }

    return NULL;
}

void *get_stack_base()
{
    pthread_t thread = pthread_self();
    void *stack_addr = pthread_get_stackaddr_np(thread);
    size_t stack_size = pthread_get_stacksize_np(thread);

    return stack_addr;
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
    uint64_t bss_size = 0;
    const struct mach_header_64 *header = (struct mach_header_64 *)get_own_image_header();
    uint8_t *bss_start = (uint8_t *)getsectiondata(header, "__DATA", "__bss", &bss_size);
    return (void *)bss_start;
}

void *get_bss_end()
{
    uint64_t bss_size = 0;
    const struct mach_header_64 *header = (struct mach_header_64 *)get_own_image_header();
    uint8_t *bss_start = (uint8_t *)getsectiondata(header, "__DATA", "__bss", &bss_size);
    return (void *)(bss_start+bss_size);
}

void *get_data_start()
{
    uint64_t data_size;
    const struct mach_header_64 *header = (struct mach_header_64 *)get_own_image_header();
    uint8_t *data_start = (uint8_t *)getsectiondata(header, "__DATA", "__data", &data_size);
    return (void *)data_start;
}

void *get_data_end()
{
    uint64_t data_size;
    const struct mach_header_64 *header = (struct mach_header_64 *)get_own_image_header();
    uint8_t *data_start = (uint8_t *)getsectiondata(header, "__DATA", "__data", &data_size);
    return (void *)(data_start + data_size);
}
