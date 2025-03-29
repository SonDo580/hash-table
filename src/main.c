#include <stdio.h>

#include "hash_table.h"

// Driver code
int main()
{
    // Create new hash table
    HashTable *ht = ht_new();

    // Insert key-value pairs
    ht_insert(ht, "name", "Son");
    ht_insert(ht, "age", "25");
    ht_insert(ht, "city", "Ha Noi");

    // Retrieve values
    printf("name: %s\n", ht_search(ht, "name")); // Should print: Son
    printf("age: %s\n", ht_search(ht, "age"));   // Should print: 25
    printf("city: %s\n", ht_search(ht, "city")); // Should print: Ha Noi

    // Overwrite existing key
    ht_insert(ht, "city", "Da Nang");
    printf("updated city: %s\n", ht_search(ht, "city")); // Should print: Da Nang

    // Delete a key
    ht_delete(ht, "age");
    printf("age after deletion: %s\n", ht_search(ht, "age")); // Should print: (null)

    // Destroy the hash table
    ht_destroy(ht);

    return 0;
}