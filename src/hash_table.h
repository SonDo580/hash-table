#ifndef HASH_TABLE_H
#define HASH_TABLE_H

// Structs
typedef struct
{
    char *key;
    char *value;
} HashTableItem;

typedef struct
{
    int base_size;         // Base number used to determine the table size
    int size;              // Total number of buckets
    int count;             // Number of items stored
    HashTableItem **items; // An array of pointers to items
} HashTable;

// Constants
static const int HT_INITIAL_BASE_SIZE = 53;   // minimum size (a prime)
static const int HT_PRIME_1 = 53;             // first prime for double hashing
static const int HT_PRIME_2 = 59;             // second prime for double hashing
static const int HT_MAX_LOAD_PERCENTAGE = 70; // trigger resize up
static const int HT_MIN_LOAD_PERCENTAGE = 10; // trigger resize down

// Core functions
HashTable *ht_new();                                               // Initialize hash table
void ht_destroy(HashTable *ht);                                    // Delete hash table
void ht_insert(HashTable *ht, const char *key, const char *value); // Insert a key-value pair
char *ht_search(HashTable *ht, const char *key);                   // Search for value by key
void ht_delete(HashTable *ht, const char *key);                    // Delete item by key

#endif