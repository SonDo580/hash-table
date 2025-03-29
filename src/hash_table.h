typedef struct
{
    char *key;
    char *value;
} ht_item;

typedef struct
{
    int base_size;
    int size;
    int count;
    ht_item **items; // an array of pointers to items
} hash_table;

// Initialize hash table
hash_table *ht_new();

// Delete hash table
void ht_del(hash_table *ht);

const HT_PRIME_1 = 53; // placeholder
const HT_PRIME_2 = 57; // placeholder

// Insert a key-value pair
void ht_insert(hash_table *ht, const char *key, const char *value);

// Search an item by key
char* ht_search(hash_table *ht, const char *key);

// Delete an item
void ht_delete(hash_table *ht, const char *key);
