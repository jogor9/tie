#ifndef TIE_MEMALLOC_H
#define TIE_MEMALLOC_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "bit.h"

MALLOC_FUNC(tie_free) static inline void *tie_malloc(size_t n, size_t sz);
MALLOC_FUNC(tie_free) static inline void *tie_calloc(size_t n, size_t sz);
static inline void *tie_realloc(void *p, size_t n, size_t sz);
MALLOC_FUNC(tie_free)
static inline void *tie_aligned_malloc(size_t a, size_t n, size_t sz);
MALLOC_FUNC(tie_free)
static inline void *tie_aligned_calloc(size_t a, size_t n, size_t sz);
static inline void tie_free(void *p);

#if MSVC == 1
#include <malloc.h>

MALLOC_FUNC(tie_free) static inline void *tie_malloc(size_t n, size_t sz)
{
        return tie_aligned_malloc(alignof(max_align_t), n, sz);
}

MALLOC_FUNC(tie_free) static inline void *tie_calloc(size_t n, size_t sz)
{
        return tie_aligned_calloc(alignof(max_align_t), n, sz);
}

static inline void *tie_realloc(void *p, size_t n, size_t sz)
{
        assert(n != 0 && sz != 0);

        if (mul_overflow(log2size, n, sz)) {
                return NULL;
        }

        return _aligned_realloc(p, n * sz, alignof(max_align_t));
}

MALLOC_FUNC(tie_free)

static inline void *tie_aligned_malloc(size_t a, size_t n, size_t sz)
{
        assert(n != 0 && sz != 0);
        assert(is_power_of_two(log2size, a));

        if (mul_overflow(log2size, n, sz)) {
                return NULL;
        }

        return _aligned_malloc(n * sz, a);
}

MALLOC_FUNC(tie_free)

static inline void *tie_aligned_calloc(size_t a, size_t n, size_t sz)
{
        assert(n != 0 && sz != 0);
        assert(is_power_of_two(log2size, a));

        return _aligned_calloc(NULL, n, sz, a);
}

static inline void tie_free(void *p)
{
        _aligned_free(p);
}

#else // #if MSVC == 1

MALLOC_FUNC(tie_free) static inline void *tie_malloc(size_t n, size_t sz)
{
        assert(n != 0 && sz != 0);

        if (mul_overflow(log2size, n, sz)) {
                return NULL;
        }

        return malloc(n * sz);
}

MALLOC_FUNC(tie_free) static inline void *tie_calloc(size_t n, size_t sz)
{
        assert(n != 0 && sz != 0);
        return calloc(n, sz);
}

static inline void *tie_realloc(void *p, size_t n, size_t sz)
{
        assert(sz != 0 && n != 0);

        if (mul_overflow(log2size, n, sz)) {
                return NULL;
        }

        return realloc(p, n * sz);
}

MALLOC_FUNC(tie_free)

static inline void *tie_aligned_malloc(size_t a, size_t n, size_t sz)
{
        assert(sz != 0 && n != 0);
        assert(is_power_of_two(log2size, a));

        if (mul_overflow(log2size, n, sz)) {
                return NULL;
        }

        return aligned_alloc(a, n * sz);
}

MALLOC_FUNC(tie_free)

static inline void *tie_aligned_calloc(size_t a, size_t n, size_t sz)
{
        void *r;

        assert(sz != 0 && n != 0);
        assert(is_power_of_two(log2size, a));

        if (mul_overflow(log2size, n, sz)) {
                return NULL;
        }

        r = aligned_alloc(a, n * sz);
        if (!r) {
                return r;
        }
        memset(r, 0, n * sz);

        return r;
}

static inline void tie_free(void *p)
{
        free(p);
}

#endif // if MSVC == 1 else

#endif
