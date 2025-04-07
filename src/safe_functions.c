#include "safe_functions.h"
#include <stdio.h>


void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *safe_calloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);
    if (ptr == NULL)
    {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *safe_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
    {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

void safe_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    free(ptr);
}