#ifndef SAFE_FUNCTIONS_H
#define SAFE_FUNCTIONS_H

#include <stdlib.h>

void *safe_malloc(size_t size);
void *safe_calloc(size_t nmemb, size_t size);
void *safe_realloc(void *ptr, size_t size);
void safe_free(void *ptr);

#endif // SAFE_FUNCTIONS_H