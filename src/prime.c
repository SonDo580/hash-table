#include <math.h>

#include "prime.h"

/**
 * Check if x is prime or not
 *
 * Returns:
 *  1   <->   prime
 *  0   <->   not prime
 * -1   <->   undefined (x < 2)
 */
int is_prime(const int x)
{
    if (x < 2)
        return -1;
    if (x < 4)
        return 1; // 2, 3
    if (x % 2 == 0)
        return 0; // even numbers greater than 2

    // Check divisibility by odd numbers from 3 to sqrt(x)
    for (int i = 3; i <= floor(sqrt((double)x)); i += 2)
    {
        if ((x % i) == 0)
            return 0;
    }

    return 1;
}

/**
 * Return the next prime after x, or x if x is prime
 * Use brute-force approach
 */
int next_prime(int x)
{
    while (is_prime(x) != 1)
    {
        x++;
    }
    return x;
}