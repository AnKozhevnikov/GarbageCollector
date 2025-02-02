#include "hashmap.h"

#include <stdlib.h>
#include <string.h>

struct HashMap hashmap_create(unsigned key_size, unsigned value_size, unsigned (*hashfunc)(const void *value))
{
    struct HashMap map;
    map.size = 0;
    map.capacity = 8;
    map.max_load_factor = 0.75;
    map.key_size = key_size;
    map.value_size = value_size;
    map.values = malloc(map.capacity * (key_size + value_size));
    map.used = calloc(map.capacity, sizeof(int));
    map.hashfunc = hashfunc;
    return map;
}

void hashmap_destruct(struct HashMap *map)
{
    free(map->values);
    free(map->used);
}

int hashmap_contains(const struct HashMap *map, const void *key)
{
    unsigned hash = map->hashfunc(key);
    unsigned i = hash % map->capacity;
    while (map->used[i])
    {
        if (!memcmp(map->values + i * (map->key_size + map->value_size), key, map->key_size))
        {
            return 1;
        }
        i = (i + 1) % map->capacity;
    }
    return 0;
}

void hashmap_rebuild(struct HashMap *map, unsigned new_capacity)
{
    void *old_values = map->values;
    int *old_used = map->used;
    unsigned old_capacity = map->capacity;

    map->capacity = new_capacity;
    map->values = malloc(map->capacity * (map->key_size + map->value_size));
    map->used = calloc(map->capacity, sizeof(int));
    map->size = 0;

    for (unsigned i = 0; i < old_capacity; i++)
    {
        if (old_used[i])
        {
            void *key = old_values + i * (map->key_size + map->value_size);
            void *value = old_values + i * (map->key_size + map->value_size) + map->key_size;
            hashmap_insert(map, key, value);
        }
    }

    free(old_values);
    free(old_used);
}

void hashmap_insert(struct HashMap *map, const void *key, const void *value)
{
    if (map->size >= map->max_load_factor * map->capacity)
    {
        hashmap_rebuild(map, 2 * map->capacity);
    }

    unsigned hash = map->hashfunc(key);
    unsigned i = hash % map->capacity;
    while (map->used[i])
    {
        i = (i + 1) % map->capacity;
    }

    memcpy(map->values + i * (map->key_size + map->value_size), key, map->key_size);
    memcpy(map->values + i * (map->key_size + map->value_size) + map->key_size, value, map->value_size);
    map->used[i] = 1;
    map->size++;
}

void hashmap_erase(struct HashMap *map, const void *key)
{
    unsigned hash = map->hashfunc(key);
    unsigned i = hash % map->capacity;
    while (map->used[i])
    {
        if (!memcmp(map->values + i * (map->key_size + map->value_size), key, map->key_size))
        {
            map->used[i] = 0;
            map->size--;
            break;
        }
        i = (i + 1) % map->capacity;
    }
}

void *hashmap_find(const struct HashMap *map, const void *key)
{
    unsigned hash = map->hashfunc(key);
    unsigned i = hash % map->capacity;
    while (map->used[i])
    {
        if (!memcmp(map->values + i * (map->key_size + map->value_size), key, map->key_size))
        {
            return map->values + i * (map->key_size + map->value_size) + map->key_size;
        }
        i = (i + 1) % map->capacity;
    }
    return NULL;
}