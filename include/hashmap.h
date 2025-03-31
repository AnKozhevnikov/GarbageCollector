#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <pthread.h>

struct HashMap
{
    unsigned size;
    unsigned capacity;
    double max_load_factor;
    unsigned key_size;
    unsigned value_size;
    void *values;
    int *used;
    int *deleted;

    unsigned (*hashfunc)(const void *value);
    pthread_rwlock_t lock;
};

struct Iterator
{
    struct HashMap *map;
    unsigned index;
    void *key;
    void *value;
};

struct HashMap hashmap_create(unsigned key_size, unsigned value_size, unsigned (*hashfunc)(const void *value));
void hashmap_destruct(struct HashMap *map);
int hashmap_contains(const struct HashMap *map, const void *key);
void hashmap_rebuild(struct HashMap *map, unsigned new_capacity);
void hashmap_insert(struct HashMap *map, const void *key, const void *value);
void hashmap_erase(struct HashMap *map, const void *key);
struct Iterator hashmap_find(const struct HashMap *map, const void *key);
struct Iterator hashmap_begin(const struct HashMap *map);
struct Iterator hashmap_next(struct Iterator it);
int hashmap_not_end(struct Iterator it);

#endif // HASHMAP_H