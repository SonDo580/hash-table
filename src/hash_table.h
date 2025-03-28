typedef struct
{
    char *key;
    char *value;
} ht_item;

typedef struct
{
    int capacity;
    int size;
    ht_item **items; // an array of pointers to items
} hash_table;

// Initialize hash table
hash_table *ht_new();

// Delete hash table
void ht_del(hash_table *ht);

const HT_PRIME_1 = 53; // placeholder
const HT_PRIME_2 = 57; // placeholder
