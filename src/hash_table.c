#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "hash_table.h"
#include "prime.h"

// Special marker for deleted items
static HashTableItem HT_DELETED_ITEM = {NULL, NULL};

// Utility: Create a new item
static HashTableItem *ht_new_item(const char *key, const char *value)
{
    HashTableItem *item = malloc(sizeof(HashTableItem));
    if (!item)
    {
        fprintf(stderr, "Error: Memory allocation failed for HashTableItem\n");
        exit(EXIT_FAILURE);
    }

    item->key = strdup(key);
    if (!item->key)
    {
        free(item);
        fprintf(stderr, "Error: Memory allocation failed for key duplication\n");
        exit(EXIT_FAILURE);
    }

    item->value = strdup(value);
    if (!item->value)
    {
        free(item->key);
        free(item);
        fprintf(stderr, "Error: Memory allocation failed for value duplication\n");
        exit(EXIT_FAILURE);
    }

    return item;
}

// Utility: Free an item
static void ht_destroy_item(HashTableItem *item)
{
    if (!item || item == &HT_DELETED_ITEM)
    {
        return;
    }

    free(item->key);
    free(item->value);
    free(item);
}

// Utility: Create hash table with the specified base size
// - base_size is used to find the next prime number,
//   which is used as the hash table size
static HashTable *ht_new_sized(const int base_size)
{
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht)
    {
        fprintf(stderr, "Error: Memory allocation failed for HashTable\n");
        exit(EXIT_FAILURE);
    }

    ht->base_size = base_size;
    ht->size = next_prime(base_size);
    ht->count = 0;

    ht->items = calloc((size_t)ht->size, sizeof(HashTableItem *));
    if (!ht->items)
    {
        fprintf(stderr, "Error: Memory allocation failed for HashTable items\n");
        free(ht);
        exit(EXIT_FAILURE);
    }

    return ht;
}

// Public: Initialize hash table
HashTable *ht_new()
{
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}

// Public: Free the hash table
void ht_destroy(HashTable *ht)
{
    if (!ht)
    {
        return;
    }

    // Free each stored item before freeing the items array
    for (int i = 0; i < ht->size; i++)
    {
        HashTableItem *item = ht->items[i];
        ht_destroy_item(item);
    }

    free(ht->items); // Free the array that holds pointers to items
    free(ht);        // Free the hash table
}

// Hash function: Map a string to an index in the hash table
// - Use a prime number as multiplier to reduce clustering
// - Ensure an even distribution of hash values for an average set of inputs
static int ht_hash(const char *str, const int prime, const int bucket_size)
{
    long hashed = 0;
    const int len_str = strlen(str);
    for (int i = 0; i < len_str; i++)
    {
        // Weighted character contribution:
        // - Multiply ASCII value of str[i] by prime^(position weight)
        // - Gives higher weight to earlier characters in the string
        hashed += (long)pow(prime, len_str - (i + 1)) * str[i];

        // Keep hash value within bucket_size
        hashed %= bucket_size;
    }

    return (int)hashed;

    // Note:
    // - Pathological inputs is the set of inputs that all hash to the same value
    // - Searching for those keys will take O(n) instead of O(1)
}

// Collision resolution: Open Addressing with Double Hashing
// - Calculate the index an item should be stored at after 'attempt' collisions
// - Avoid clustering issue seen in linear probing
static int ht_get_index(
    const char *str, const int bucket_size, const int attempt)
{
    const int hash_a = ht_hash(str, HT_PRIME_1, bucket_size);
    const int hash_b = ht_hash(str, HT_PRIME_2, bucket_size);

    // Avoid getting stuck in a cycle by adding 1 to `hash_b`
    return (hash_a + (attempt * (hash_b + 1))) % bucket_size;
}

// Utility: Calculate load factor (filled buckets / total buckets)
// - use percentage to avoid floating point math
static int load_percentage(HashTable *ht)
{
    return ht->count * 100 / ht->size;
}

// Utility: Resize hash table
static void ht_resize(HashTable *ht, const int base_size)
{
    // Don't reduce the size below the minimum
    if (base_size < HT_INITIAL_BASE_SIZE)
    {
        return;
    }

    // Create a new hash table with updated size
    HashTable *new_ht = ht_new_sized(base_size);

    // Insert existing items into the new hash table
    for (int i = 0; i < ht->size; i++)
    {
        HashTableItem *item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM)
        {
            ht_insert(new_ht, item->key, item->value);
        }
    }

    // Give new_ht ht's size and items, then delete new_ht
    // (free memory used by the old ht)
    const int temp_size = ht->size;
    HashTableItem **temp_items = ht->items;

    ht->size = new_ht->size;
    ht->items = new_ht->items;
    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    new_ht->size = temp_size;
    new_ht->items = temp_items;
    ht_destroy(new_ht);
}

// Helper: resize up when load is high
static void ht_resize_up(HashTable *ht)
{
    const int new_base_size = ht->base_size * 2;
    ht_resize(ht, new_base_size);
}

// Helper: resize down when load is low
static void ht_resize_down(HashTable *ht)
{
    const int new_base_size = ht->base_size / 2;
    ht_resize(ht, new_base_size);
}

// Public: Insert a key-value pair
void ht_insert(HashTable *ht, const char *key, const char *value)
{
    // Resize up if needed
    const int load = load_percentage(ht);
    if (load > HT_MAX_LOAD_PERCENTAGE)
    {
        ht_resize_up(ht);
    }

    // Create a new item
    HashTableItem *item = ht_new_item(key, value);

    int index = ht_get_index(item->key, ht->size, 0);
    HashTableItem *existing_item = ht->items[index];
    int attempt = 1;
    while (existing_item != NULL)
    {
        // Overwrite value if the key already exists
        if (existing_item != &HT_DELETED_ITEM && strcmp(existing_item->key, key) == 0)
        {
            ht_destroy_item(existing_item);
            ht->items[index] = item;
            return;
        }

        // Probe for the next index if collision occurs
        index = ht_get_index(item->key, ht->size, attempt);
        existing_item = ht->items[index];
        attempt++;
    }

    // Insert the new item
    ht->items[index] = item;
    ht->count++;
}

// Public: Search value by key
char *ht_search(HashTable *ht, const char *key)
{
    int index = ht_get_index(key, ht->size, 0);
    HashTableItem *item = ht->items[index];
    int attempt = 1;
    while (item != NULL)
    {
        // If key is found and not marked as deleted, return the value
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            return item->value;
        }

        // Continue probing
        index = ht_get_index(key, ht->size, attempt);
        item = ht->items[index];
        attempt++;
    }

    return NULL; // Not found
}

// Public: Delete an item by key
// - We cannot remove the item, as it will break the collision chain
// - Just mark it as deleted
void ht_delete(HashTable *ht, const char *key)
{
    // Resize down if needed
    const int load = load_percentage(ht);
    if (load < HT_MIN_LOAD_PERCENTAGE)
    {
        ht_resize_down(ht);
    }

    int index = ht_get_index(key, ht->size, 0);
    HashTableItem *item = ht->items[index];
    int attempt = 1;
    while (item != NULL)
    {
        // If the item is found, perform the deletion
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            ht_destroy_item(item);
            ht->items[index] = &HT_DELETED_ITEM;
            ht->count--;
            return;
        }

        // Continue probing
        index = ht_get_index(key, ht->size, attempt);
        item = ht->items[index];
        attempt++;
    }
}
