#ifndef TIE_NUMERIC_H
#define TIE_NUMERIC_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "attrib.h"
#include "base_array.h"
#include "functional.h"

#define max_by(expr, a, b) ((expr) > 0 ? (a) : (b))
#define min_by(expr, a, b) ((expr) < 0 ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define sgn(x) ((x) > 0 ? 1 : (x) < 0 ? -1 : 0)
#define is_even(x) ((x) % 2 == 0)
#define is_odd(x) (!is_even(x))

extern PURE_FUNC size_t max_factorial(size_t size_bytes);

#define factorial_decl(type, name)                                             \
        /* it's pure not const because of the max_factorial assert */          \
        PURE_FUNC static inline type factorial_##name(type n)                  \
        {                                                                      \
                type i, r = 1;                                                 \
                                                                               \
                assert(n <= max_factorial(sizeof(n)));                         \
                                                                               \
                traverse(i, 2, n + 1)                                          \
                        r *= i;                                                \
                return r;                                                      \
        }                                                                      \
        PURE_FUNC static inline type factorial_##name(type n)

factorial_decl(uint8_t, uint8);
factorial_decl(uint16_t, uint16);
factorial_decl(uint32_t, uint32);
factorial_decl(uint64_t, uint64);
factorial_decl(size_t, size);

#endif
