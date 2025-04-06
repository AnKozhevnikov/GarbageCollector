#include <gtest/gtest.h>

extern "C"
{
#include "gc.h"
}

TEST(HashMap, create_destruct)
{
    struct HashMap *map = hashmap_create(sizeof(int), sizeof(int), hash_for_pointer);
    hashmap_destruct(map);
    SUCCEED();
}

TEST(HashMap, contains)
{
    struct HashMap *map = hashmap_create(sizeof(int), sizeof(int), hash_for_pointer);
    int key = 1;
    hashmap_insert(map, &key, &key);
    EXPECT_TRUE(hashmap_contains(map, &key));
    hashmap_destruct(map);
}

TEST(HashMap, insert_erase)
{
    struct HashMap *map = hashmap_create(sizeof(int), sizeof(int), hash_for_pointer);
    int key = 1;
    hashmap_insert(map, &key, &key);
    EXPECT_TRUE(hashmap_contains(map, &key));
    hashmap_erase(map, &key);
    EXPECT_FALSE(hashmap_contains(map, &key));
    hashmap_destruct(map);
}

TEST(HashMap, find)
{
    struct HashMap *map = hashmap_create(sizeof(int), sizeof(int), hash_for_pointer);
    int key = 1;
    hashmap_insert(map, &key, &key);
    struct Iterator it = hashmap_find(map, &key);
    EXPECT_TRUE(hashmap_not_end(it));
    hashmap_destruct(map);
}

TEST(HashMap, begin_next)
{
    struct HashMap *map = hashmap_create(sizeof(int), sizeof(int), hash_for_pointer);
    int key = 1;
    hashmap_insert(map, &key, &key);
    struct Iterator it = hashmap_begin(map);
    EXPECT_TRUE(hashmap_not_end(it));
    it = hashmap_next(it);
    EXPECT_FALSE(hashmap_not_end(it));
    hashmap_destruct(map);
}

TEST(HashMap, big_usecase)
{
    struct HashMap *map = hashmap_create(sizeof(int), sizeof(int), hash_for_pointer);
    for (int i = 0; i < 1000; i++)
    {
        hashmap_insert(map, &i, &i);
    }
    for (int i = 0; i < 1000; i++)
    {
        EXPECT_TRUE(hashmap_contains(map, &i));
    }
    for (int i = 0; i < 1000; i++)
    {
        hashmap_erase(map, &i);
    }
    for (int i = 0; i < 1000; i++)
    {
        EXPECT_FALSE(hashmap_contains(map, &i));
    }
    hashmap_destruct(map);
}

struct HashMap *map;
int keys[1000];
int values[1000];

void *inserter(void *arg)
{
    int num = *(int *)arg;
    for (int i = num * 100; i < num * 100 + 100; i++)
    {
        hashmap_insert(map, &keys[i], &values[i]);
    }
    return 0;
}

TEST(HashMap, multi_thread)
{
    for (int i = 0; i < 1000; i++)
    {
        keys[i] = i;
    }

    for (int i = 0; i < 1000; i++)
    {
        values[i] = rand();
    }

    map = hashmap_create(sizeof(int), sizeof(int), hash_for_pointer);

    pthread_t threads[10];
    int oneten[10];
    for (int i = 0; i < 10; i++)
    {
        oneten[i] = i;
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&threads[i], 0, inserter, &oneten[i]);
    }

    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], 0);
    }

    for (int i = 0; i < 1000; i++)
    {
        ASSERT_TRUE(hashmap_contains(map, &keys[i]));
    }
}