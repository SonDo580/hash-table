#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

// Initialize item
static ht_item *ht_new_item(const char *key, const char *value)
{
    ht_item *item = malloc(sizeof(ht_item));
    item->key = strdup(key);
    item->value = strdup(value);
    return item;

    // Note: Save copies of 'key' and 'value'
}

// Initialize hash table
hash_table *ht_new()
{
    hash_table *ht = malloc(sizeof(hash_table));
    ht->capacity = 53;
    ht->size = 0;
    ht->items = calloc((size_t)ht->capacity, sizeof(ht_item *));
    return ht;

    // Note:
    // 1. Why use 53 for capacity:
    // - Prime numbers are often used for hash table sizes to reduce collisions.
    // - It prevents issues with certain hash functions that may distribute keys poorly.
    //
    // 2. Why use calloc() instead of malloc():
    // - It initializes all allocated memory to NULL (zero-out memory).
    // - This prevents dangling pointers in the ht->items array.
}

// Delete item
static void ht_del_item(ht_item *item)
{
    free(item->key);
    free(item->value);
    free(item);
}

// Delete hash table
void ht_del(hash_table *ht)
{
    for (int i = 0; i < ht->capacity; i++)
    {
        ht_item *item = ht->items[i];
        if (item != NULL)
        {
            ht_del_item(item);
        }
    }

    free(ht->items);
    free(ht);
}

// Hash function
// - Take a string as input and return a number between 0 and m
//   (the desired bucket array length)
// - Return an even distribution of bucket indices for an average set of inputs
static int hash_fn(const char *str, const int prime, const int bucket_size)
{
    long hashed = 0;
    const int len_str = strlen(str);
    for (int i = 0; i < len_str; i++)
    {
        // Raise prime to a decreasing exponent, then multiply by the ASCII value of str[i].
        // (This gives greater weights for earlier characters in the string)
        hashed += (long)pow(prime, len_str - (i + 1)) * str[i];

        // Keep hashed within bucket_size
        hashed %= bucket_size;
    }

    return (int)hashed;

    // Pathological inputs:
    // - The set of inputs that all hash to the same value
    // - Searching for those keys will take O(n) instead of O(1)
}

// Handle collision
// Technique: Open addressing with double hashing
// - Calculate the index an item should be stored at after 'attempt' collisions
// - We plus 1 to hash_b to avoid getting the same index over and over again.
static int ht_get_hash(
    const char *str, const int bucket_size, const int attempt)
{
    const int hash_a = hash_fn(str, HT_PRIME_1, bucket_size);
    const int hash_b = hash_fn(str, HT_PRIME_2, bucket_size);
    return (hash_a + (attempt * (hash_b + 1))) % bucket_size;
}

// Insert a key-value pair
// - Iterate through the indices until we find an empty bucket
// - Insert the item into that bucket and increment hash table's size
void ht_insert(hash_table *ht, const char *key, const char *value)
{
    ht_item *item = ht_new_item(key, value);
    int index = ht_get_hash(item->key, ht->capacity, 0);

    ht_item *existing_item = ht->items[index];
    int attempt = 1;
    while (existing_item != NULL)
    {
        // Overwrite value to the same key (don't increment size)
        if (existing_item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
            ht_del_item(existing_item);
            ht->items[index] = item;
            return;
        }

        index = ht_get_hash(item->key, ht->capacity, attempt);
        existing_item = ht->items[index];
        attempt++;
    }

    // Insert the new item
    ht->items[index] = item;
    ht->size++;
}

// Search value by key
// - Calculate the index and compare item key with search key
// - If the keys are equal, return the item value
// - If hitting a NULL bucket, return NULL (item not found)
char *ht_search(hash_table *ht, const char *key)
{
    int index = ht_get_hash(key, ht->capacity, 0);
    ht_item *item = ht->items[index];

    int attempt = 1;
    while (item != NULL)
    {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            // Return the value
            return item->value;
        }

        index = ht_get_hash(key, ht->size, attempt);
        item = ht->items[index];
        attempt++;
    }

    return NULL;
}

// Delete an item
// - We cannot simply remove the item, as it will break the collision chain.
// - Just mark the item as deleted
static ht_item HT_DELETED_ITEM = {NULL, NULL};

void ht_delete(hash_table *ht, const char *key)
{
    int index = ht_get_hash(key, ht->size, 0);
    ht_item *item = ht->items[index];

    int attempt = 1;
    while (item != NULL)
    {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            ht_del_item(item);
            ht->items[index] = &HT_DELETED_ITEM;
            ht->size--;
            return;
        }

        index = ht_get_hash(key, ht->size, attempt);
        attempt++;
    }
}