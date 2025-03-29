# Hash table implementation

- Technique: open addressing with double hashing
- Limitation: only handle ASCII strings for key and value
- Guide: https://github.com/jamesroutley/write-a-hash-table

## Other collision handling methods:

- Separate chaining
- Open addressing with linear probing
- Open addressing with quadratic probing

## Run driver code

1. Build the project

```bash
make # build
```

2. Run the executable

```bash
./build/main # run on Linux
```

## Questions:

```c
static int ht_hash(const char *str, const int prime, const int bucket_size)
{
    long hashed = 0;
    const int len_str = strlen(str);
    for (int i = 0; i < len_str; i++)
    {
        hashed += (long)pow(prime, len_str - (i + 1)) * str[i];
        hashed %= bucket_size;
    }
    return (int)hashed;
}
```

1. **Why use weighted character hashing?**

- Reduce similar strings hash to the same value
  - if there's no weight factor, `ht_hash("abc")` and `ht_hash("bca")` will hash to the same value.
  - by using position weights, we ensure that swapping letters change the hash.

2. **Why use a prime number as multiplier in the hash function?**

- Distribute the hash values evenly across the range
  - ... need more research

3. **Why use a prime number for bucket size?**

- Ensure all buckets are reachable
  - ... need more research
