#include "hashmap.h"

#include <stdlib.h>
#include <string.h>

struct HashMap hashmap_create(unsigned key_size, unsigned value_size, unsigned (*hashfunc)(const void *value))
{
    struct HashMap map;
    map.size = 0;
    map.deleted_cnt = 0;
    map.capacity = 8;
    map.max_load_factor = 0.75;
    map.key_size = key_size;
    map.value_size = value_size;
    map.values = malloc(map.capacity * (key_size + value_size));
    map.used = calloc(map.capacity, sizeof(int));
    map.deleted = calloc(map.capacity, sizeof(int));
    map.hashfunc = hashfunc;
    pthread_rwlock_init(&map.lock, 0);
    return map;
}

void hashmap_destruct(struct HashMap *map)
{
    free(map->values);
    free(map->used);
    free(map->deleted);
    pthread_rwlock_destroy(&map->lock);
}

int hashmap_contains(const struct HashMap *map, const void *key)
{
    pthread_rwlock_rdlock(&map->lock);

    unsigned hash = map->hashfunc(key);
    unsigned i = hash % map->capacity;
    while (map->used[i])
    {
        if (!memcmp(map->values + i * (map->key_size + map->value_size), key, map->key_size) && !map->deleted[i])
        {
            pthread_rwlock_unlock(&map->lock);
            return 1;
        }
        i = (i + 1) % map->capacity;
    }

    pthread_rwlock_unlock(&map->lock);
    return 0;
}

void hashmap_rebuild(struct HashMap *map, unsigned new_capacity)
{
    void *old_values = map->values;
    int *old_used = map->used;
    int *old_deleted = map->deleted;
    unsigned old_capacity = map->capacity;

    map->capacity = new_capacity;
    map->values = malloc(map->capacity * (map->key_size + map->value_size));
    map->used = calloc(map->capacity, sizeof(int));
    map->deleted = calloc(map->capacity, sizeof(int));
    map->size = 0;
    map->deleted_cnt = 0;

    for (unsigned i = 0; i < old_capacity; i++)
    {
        if (old_used[i] && !old_deleted[i])
        {
            void *key = old_values + i * (map->key_size + map->value_size);
            void *value = old_values + i * (map->key_size + map->value_size) + map->key_size;
            hashmap_insert_nolock(map, key, value);
        }
    }

    free(old_values);
    free(old_used);
    free(old_deleted);
}

void hashmap_insert_nolock(struct HashMap *map, const void *key, const void *value)
{
    if (map->size + map->deleted_cnt >= map->max_load_factor * map->capacity)
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

void hashmap_insert(struct HashMap *map, const void *key, const void *value)
{
    pthread_rwlock_wrlock(&map->lock);

    if (map->size + map->deleted_cnt >= map->max_load_factor * map->capacity)
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

    pthread_rwlock_unlock(&map->lock);
}

void hashmap_erase(struct HashMap *map, const void *key)
{
    pthread_rwlock_wrlock(&map->lock);

    unsigned hash = map->hashfunc(key);
    unsigned i = hash % map->capacity;
    while (map->used[i])
    {
        if (!memcmp(map->values + i * (map->key_size + map->value_size), key, map->key_size) && !map->deleted[i])
        {
            map->deleted[i] = 1;
            map->size--;
            map->deleted_cnt++;
            break;
        }
        i = (i + 1) % map->capacity;
    }

    pthread_rwlock_unlock(&map->lock);
}

struct Iterator hashmap_find(const struct HashMap *map, const void *key)
{
    pthread_rwlock_rdlock(&map->lock);

    unsigned hash = map->hashfunc(key);
    unsigned i = hash % map->capacity;
    while (map->used[i])
    {
        if (!memcmp(map->values + i * (map->key_size + map->value_size), key, map->key_size) && !map->deleted[i])
        {
            struct Iterator ret;
            ret.map = map;
            ret.index = i;
            ret.key = map->values + i * (map->key_size + map->value_size);
            ret.value = map->values + i * (map->key_size + map->value_size) + map->key_size;
            pthread_rwlock_unlock(&map->lock);
            return ret;
        }
        i = (i + 1) % map->capacity;
    }

    pthread_rwlock_unlock(&map->lock);

    return (struct Iterator){0};
}

struct Iterator hashmap_begin(const struct HashMap *map)
{
    pthread_rwlock_rdlock(&map->lock);

    for (unsigned i = 0; i < map->capacity; i++)
    {
        if (map->used[i] && !map->deleted[i])
        {
            struct Iterator ret;
            ret.map = map;
            ret.index = i;
            ret.key = map->values + i * (map->key_size + map->value_size);
            ret.value = map->values + i * (map->key_size + map->value_size) + map->key_size;
            pthread_rwlock_unlock(&map->lock);
            return ret;
        }
    }

    pthread_rwlock_unlock(&map->lock);

    return (struct Iterator){0};
}

struct Iterator hashmap_next(struct Iterator it)
{
    pthread_rwlock_rdlock(&it.map->lock);

    for (unsigned i = it.index + 1; i < it.map->capacity; i++)
    {
        if (it.map->used[i] && !it.map->deleted[i])
        {
            struct Iterator ret;
            ret.map = it.map;
            ret.index = i;
            ret.key = it.map->values + i * (it.map->key_size + it.map->value_size);
            ret.value = it.map->values + i * (it.map->key_size + it.map->value_size) + it.map->key_size;
            pthread_rwlock_unlock(&it.map->lock);
            return ret;
        }
    }

    pthread_rwlock_unlock(&it.map->lock);

    return (struct Iterator){0};
}

int hashmap_not_end(struct Iterator it)
{
    return it.map != 0;
}
