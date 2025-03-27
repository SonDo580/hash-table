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