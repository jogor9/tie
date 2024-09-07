#ifndef TIE_RANDOM_H
#define TIE_RANDOM_H

#include <string.h>

#include "array.h"
#include "bit.h"
#include "numeric.h"

// todo: versions that make sure all of the output numbers/ranges are unique
#define random_in_range(begin, end, rng, rng_state)                            \
        ((begin) + (rng)(rng_state) % ((end) - (begin)))
#define shuffle(p, begin, end, temp, rng, rng_state)                           \
        traverse(p, begin, (end) - 1)                                          \
        swap(p, random_in_range(p, end, rng, rng_state), temp)
#define xor_shuffle(p, begin, end, rng, rng_state)                             \
        traverse(p, begin, (end) - 1)                                          \
        xor_swap(p, random_in_range(p, end, rng, rng_state))
#define for_random_in_range(p, begin, end, n, i, rng, rng_state)               \
        for ((i) = 0;                                                          \
             (i) < (n)                                                         \
             && ((p) = random_in_range(begin, end, rng, rng_state), 1);        \
             ++(i))
#define random_permutations(out, begin, end, permut_fn, n, i, rng, rng_state)  \
        for ((i) = 0;                                                          \
             (i) < (n)                                                         \
             && (memcpy(out, begin, range_bytes(begin, end)),                  \
                 (permut_fn)((end) - (begin),                                  \
                             out,                                              \
                             random_in_range(0,                                \
                                             factorial_size((end) - (begin)),  \
                                             rng,                              \
                                             rng_state)),                      \
                 1);                                                           \
             ++(i))

static inline uint32_t lcg64(uint64_t *restrict seed)
{
        const uint64_t p = 6364136223846793005;
        const uint64_t off = 1;
        *seed = p * *seed + off;
        return (uint32_t)(*seed >> 32);
}

/* https://prng.di.unimi.it/ */

static inline uint64_t splitmix64(uint64_t *restrict s)
{
        uint64_t r;

        *s += 0x9E3779B97f4A7C15;
        r = *s;
        r = (r ^ (r >> 30)) * 0xBF58476D1CE4E5B9;
        r = (r ^ (r >> 27)) * 0x94D049BB133111EB;
        return r ^ (r >> 31);
}

typedef struct {
        uint64_t s[4];
} xoshiro256ss_state;

static inline uint64_t xoshiro256ss(xoshiro256ss_state *restrict s)
{
        uint64_t r, t;

        r = s->s[0] + s->s[3];
        t = s->s[1] << 17;

        s->s[2] ^= s->s[0];
        s->s[3] ^= s->s[1];
        s->s[1] ^= s->s[2];
        s->s[0] ^= s->s[3];

        s->s[2] ^= t;
        s->s[3] = rotatel(s->s[3], 45);

        return r;
}

static inline void xoshiro256_init(uint64_t seed, uint64_t s[static restrict 4])
{
        s[0] = splitmix64(&seed);
        s[1] = splitmix64(&seed);
        s[2] = splitmix64(&seed);
        s[3] = splitmix64(&seed);
}

typedef struct {
        uint64_t s[4];
} xoshiro256p_state;

static inline double xoshiro256p(xoshiro256p_state *restrict s)
{
        double r = norm_u64_to_double(s->s[0] + s->s[3]);
        uint64_t t = s->s[1] << 17;

        s->s[2] ^= s->s[0];
        s->s[3] ^= s->s[1];
        s->s[1] ^= s->s[2];
        s->s[0] ^= s->s[3];

        s->s[2] ^= t;

        s->s[3] = rotatel(s->s[3], 45);

        return r;
}

#endif
