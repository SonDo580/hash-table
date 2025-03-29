#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "prime.h"

// Initialize item
static ht_item *ht_new_item(const char *key, const char *value)
{
    ht_item *item = malloc(sizeof(ht_item));
    item->key = strdup(key);
    item->value = strdup(value);
    return item;

    // Note: Save copies of 'key' and 'value'
}

static hash_table *ht_new_sized(const int base_size)
{
    hash_table *ht = xmalloc(sizeof(hash_table));
    ht->base_size = base_size;
    ht->size = next_prime(base_size);
    ht->count = 0;
    ht->items = xcalloc((size_t)ht->size, sizeof(ht_item *));
    return ht;
}

// Initialize hash table
static int HT_INITIAL_BASE_SIZE = 53;
// Why use 53:
// - Prime numbers are often used for hash table sizes to reduce collisions.
// - It prevents issues with certain hash functions that may distribute keys poorly.

static hash_table *ht_new()
{
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
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
    for (int i = 0; i < ht->size; i++)
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
    // Resize up if the load is above 0.7
    const int load = load_percentage(ht);
    if (load > 70)
    {
        ht_resize_up(ht);
    }

    ht_item *item = ht_new_item(key, value);
    int index = ht_get_hash(item->key, ht->size, 0);

    ht_item *existing_item = ht->items[index];
    int attempt = 1;
    while (existing_item != NULL)
    {
        // Overwrite value to the same key
        if (existing_item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            ht_del_item(existing_item);
            ht->items[index] = item;
            return;
        }

        index = ht_get_hash(item->key, ht->size, attempt);
        existing_item = ht->items[index];
        attempt++;
    }

    // Insert the new item
    ht->items[index] = item;
    ht->count++;
}

// Search value by key
// - Calculate the index and compare item key with search key
// - If the keys are equal, return the item value
// - If hitting a NULL bucket, return NULL (item not found)
char *ht_search(hash_table *ht, const char *key)
{
    int index = ht_get_hash(key, ht->size, 0);
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
    // Resize down if the load is below 0.1
    const int load = load_percentage(ht);
    if (load < 10)
    {
        ht_resize_down(ht);
    }

    int index = ht_get_hash(key, ht->size, 0);
    ht_item *item = ht->items[index];

    int attempt = 1;
    while (item != NULL)
    {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            ht_del_item(item);
            ht->items[index] = &HT_DELETED_ITEM;
            ht->count--;
            return;
        }

        index = ht_get_hash(key, ht->size, attempt);
        attempt++;
    }
}

// Resize hash table
static void ht_resize(hash_table *ht, const int base_size)
{
    // Don't reduce the size below the minimum
    if (base_size < HT_INITIAL_BASE_SIZE)
    {
        return;
    }

    hash_table *new_ht = ht_new_sized(base_size);
    for (int i = 0; i < ht->size; i++)
    {
        ht_item *item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM)
        {
            ht_insert(new_ht, item->key, item->value);
        }
    }

    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    // Give new_ht ht's size and items then delete it
    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;

    ht_item **tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    ht_del(new_ht);
}

static void ht_resize_up(hash_table *ht)
{
    const int new_base_size = ht->base_size * 2;
    ht_resize(ht, new_base_size);
}

static void ht_resize_down(hash_table *ht)
{
    const int new_base_size = ht->base_size / 2;
    ht_resize(ht, new_base_size);
}

// - load: filled buckets / total buckets
// - use percentage to avoid doing floating point math
static int load_percentage(hash_table *ht)
{
    return ht->count * 100 / ht->size;
}