#ifndef TIE_BIT_ALGO_H
#define TIE_BIT_ALGO_H

#include <stddef.h>
#include <stdint.h>

#include "attrib.h"

#define bit_count(x) (sizeof(x) * 8)

// Returns a uintmax_t mask containing bits in range [i, j]
#define mask(i, j) ((((uintmax_t)1 << ((j) - (i) + 1)) - 1) << (i))
// Returns a uintmax_t mask containing bits in range [i, j] from the end
#define rmask(a, i, j) mask(bit_count(a) - 1 - (i), bit_count(a) - 1 - (j))
// Returns a uintmax_t mask containing one bit at index i
#define bit(i) mask(i, i)
// Returns a uintmax_t mask containing one bit at index i from the end
#define rbit(a, i) rmask(a, i, i)

#define rotatel(x, s) ((x) << (s) | (x) >> (bit_count(x) - s))
#define rotater(x, s) ((x) >> (s) | (x) << (bit_count(x) - s))

#define xor_swap(a, b) ((a) ^= (b), (b) ^= (a), (a) ^= (b))
#define ptr_swap(a, b)                                                         \
        ((a) = (void *)((uintptr_t)(a) ^ (uintptr_t)(b)),                      \
         (b) = (void *)((uintptr_t)(b) ^ (uintptr_t)(a)),                      \
         (a) = (void *)((uintptr_t)(a) ^ (uintptr_t)(b)))

#define ilog2_decl(type, funname)                                              \
        CONST_FUNC static inline int funname(type n)                           \
        {                                                                      \
                int i = -1;                                                    \
                while (n != 0) {                                               \
                        ++i;                                                   \
                        n >>= 1;                                               \
                }                                                              \
                return i;                                                      \
        }                                                                      \
        CONST_FUNC static inline int funname(type n)
#define ilog2_ceil(log2int, n) (log2int(n) + (n != ipow2(log2int(n))))

ilog2_decl(uint8_t, log2uint8);
ilog2_decl(uint16_t, log2uint16);
ilog2_decl(uint32_t, log2uint32);
ilog2_decl(uint64_t, log2uint64);
ilog2_decl(size_t, log2size);

#define div_ceil(a, b) ((a) / (b) + ((a) % (b) != 0))

// overestimates the multiplication result
// but at that point might as well fail
// on 32-bit at worst fails when multiplying 2^15 + 1 by 2^15 + 1
// which is 2^30 + 2^16 + 1
#define mul_overflow(log2int, a, b)                                            \
        (ilog2_ceil(log2int, a) + ilog2_ceil(log2int, b) >= (int)bit_count(a))
#define ipow2(n) bit(n)
#define is_power_of_two(log2int, x) (ipow2(log2int(x)) == (uintmax_t)(x))

#define norm_u8_to_single(x) ((x) * 0x1p-8f)
#define norm_u16_to_single(x) ((x) * 0x1p-16f)
#define norm_u32_to_single(x) (((x) >> 9) * 0x1p-23f)
#define norm_u64_to_single(x) (((x) >> 41) * 0x1p-23f)
#define norm_u8_to_double(x) ((x) * 0x1p-8)
#define norm_u16_to_double(x) ((x) * 0x1p-16)
#define norm_u32_to_double(x) ((x) * 0x1p-32)
#define norm_u64_to_double(x) (((x) >> 11) * 0x1p-53)

#endif
