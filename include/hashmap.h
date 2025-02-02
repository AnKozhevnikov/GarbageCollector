#pragma once

#include <stddef.h>

struct HashMap {
    unsigned size;
    unsigned capacity;
    double max_load_factor;
    unsigned key_size;
    unsigned value_size;
    void *values;
    int *used;

    unsigned (*hashfunc)(const void *value);
};

struct HashMap hashmap_create(unsigned key_size, unsigned value_size, unsigned (*hashfunc)(const void *value));
void hashmap_destruct(struct HashMap *map);
int hashmap_contains(const struct HashMap *map, const void *key);
void hashmap_rebuild(struct HashMap *map, unsigned new_capacity);
void hashmap_insert(struct HashMap *map, const void *key, const void *value);
void hashmap_erase(struct HashMap *map, const void *key);
void *hashmap_find(const struct HashMap *map, const void *key);