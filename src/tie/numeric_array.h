#ifndef TIE_NUMERIC_ARRAY_H
#define TIE_NUMERIC_ARRAY_H

#include "base_array.h"
#include "numeric.h"

#define shift_left(p, src, dst, temp)                                          \
        rtraverse(p, dst, src)                                                 \
        swap(p[0], p[1], temp)
#define shift_right(p, src, dst, temp)                                         \
        traverse(p, src, dst)                                                  \
        swap(p[0], p[1], temp)
#define xor_shift_left(p, src, dst)                                            \
        rtraverse(p, dst, src)                                                 \
        xor_swap(p[0], p[1])
#define xor_shift_right(p, src, dst)                                           \
        traverse(p, src, dst)                                                  \
        xor_swap(p[0], p[1])

#define ith_permutation_decl(type, funname)                                    \
        static inline void funname(size_t i,                                   \
                                   size_t n,                                   \
                                   type v[static restrict n])                  \
        {                                                                      \
                size_t fac;                                                    \
                type temp, *p, *q;                                             \
                                                                               \
                assert(n > 0 && n <= max_factorial(sizeof(*v)));               \
                                                                               \
                fac = factorial_size(n);                                       \
                                                                               \
                assert(i < fac);                                               \
                                                                               \
                fac /= n;                                                      \
                traverse(p, v, v + n) {                                        \
                        shift_left(q, p + i / fac, p, temp);                   \
                        i %= fac;                                              \
                        fac /= n - (p - v) - 1;                                \
                }                                                              \
        }                                                                      \
        static inline void funname(size_t i,                                   \
                                   size_t n,                                   \
                                   type v[static restrict n])

#endif
