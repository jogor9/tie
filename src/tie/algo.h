/*! \file algo.h
 *  \brief Meta header for *_algo.h headers
 */

#ifndef TIE_ALGO_H
#define TIE_ALGO_H

#include "bit.h"
#include "memalloc.h"
#include "numeric.h"

/*! \brief Function typedef for memory reallocators.
 *
 *  \sa #auxiliary_realloc() auxiliary_reallocator()
 */
typedef size_t Reallocator(void **restrict, size_t, size_t, size_t, void *);

/*! \brief Macro for algorithms that may need to allocate additional space.
 *
 *  Algorithms that wish to allocate additional space should use the following
 *  macro whenever they run out of space. In most cases the user should supply
 *  an initial data structure, hopefully with enough preallocated space for the
 *  algorithm to run freely. If the user underestimated the memory usage,
 *  however, the algorithm can simply ask the reallocator to allocate the data
 *  structure again. This macro is a convenient way to make a single-call
 *  reallocation the moment the algorithm detects that the supplied data
 *  structure was insufficient.
 *
 *  The macro accepts two pairs of variables: one owned by the user and the
 *  other owned by the algorithm itself. The pair consists
 *  of a pointer to the data structure and its maximum capacity. On failure,
 *  the algorithm's variables are invalidated; the user's variables are left
 *  intact.
 *
 *  \param[in] reallocator The reallocator provided by the user. May be null, in
 *  that case the reallocation fails immediately.
 *  \param[in,out] sz A pointer to the algorithm's variable representing the
 *  size of the data structure. Invalidated on failure.
 *  \param[out] usz A pointer to the user's variable representing the size of
 *  the data structure. Untouched on failure.
 *  \param[in,out] arr A pointer to the algorithm's variable representing the
 *  data structure to reallocate. Invalidated on failure.
 *  \param[out] uarr A pointer to the user's variable representing the provided
 *  data structure. Untouched on failure.
 *  \param[in] newsz The requested new size of the data structure. Required to
 *  be larger than `sz`. `sz` is not guaranteed to be equal to `newsz` after
 *  a successful reallocation, but it is required to be at least as big as
 *  `newsz`.
 *  \param[in,out] user Private data to pass onto the provided reallocator.
 *
 *  \return The macro is a single expression that returns a truthful value on
 *  success and a false value on failure.
 *
 *  #### Example:
 *
 *  ~~~{.c}
 *  int gen_primes_in_range(int begin, int end, int *restrict *pout,
 *                           size_t *psize, Reallocator *reallocator,
 *                           void *user)
 *  {
 *          int *out = *pout;
 *          size_t size = *psize, count = 0;
 *
 *          for (; begin < end; ++begin) {
 *                  if (!is_prime(begin)) {
 *                          continue;
 *                  }
 *                  if (count == size
 *                      && !auxiliary_realloc(reallocator, &size, &out, psize,
 *                                            pout, size + 1, user)) {
 *                          return -1;
 *                  }
 *                  out[count] = begin;
 *                  ++count;
 *          }
 *
 *          return count;
 *  }
 *  ~~~
 *
 *  \remark Because the reallocator needs to know the size of the underlying
 *  data structure, passing pointers to incomplete types will throw a compiler
 *  error. Use #auxiliary_realloc_void() if you need to pass in an incomplete
 *  type.
 *
 *  \sa auxiliary_reallocator(), #auxiliary_realloc_void()
 */
#define auxiliary_realloc(reallocator, sz, arr, usz, uarr, newsz, user)        \
        ((reallocator)                                                         \
         && (*(sz) = (reallocator)((void **)arr,                               \
                                   *(sz),                                      \
                                   newsz,                                      \
                                   sizeof(**(arr)),                            \
                                   user),                                      \
             arr)                                                              \
         && (*(uarr) = *(arr), *(usz) = *(sz), 1))
/*! This is a version of #auxiliary_realloc() that doesn't require the
 *  underlying data structure to be complete. The size passed to the allocator
 *  is zero, otherwise this macro is the same as #auxiliary_realloc().
 *
 *  \sa #auxiliary_realloc()
 */
#define auxiliary_realloc_void(reallocator, sz, arr, psz, parr, newsz, user)   \
        ((reallocator)                                                         \
         && (*(sz) = (reallocator)(arr, *(sz), newsz, 0, user), arr)           \
         && (*(parr) = *(arr), *(psz) = *(sz), 1))

/*! \brief A basic reallocator for algorithms that need dynamic memory.
 *
 *  Reallocators take a generic pointer to an array, its previous length n and
 *  its desired new length new_n and allocate at least, but not necessarily
 *  exactly new_n elements of size sz. On success, they return the actual new
 *  size of the array and write the pointer to the array back. On failure,
 *  they nullify the pointer.
 *
 *  This instance of a reallocator assumes the input data structure is an array
 *  that comes from the standard tie_*alloc() family of routines and reallocates
 *  them using tie_realloc() with 3/2 as much space for elements.
 *
 *  \param[in,out] p Pointer to the data structure. Set to NULL on failure,
 *  \param[in] n Old size of the data structure.
 *  \param[in] new_n The requested size of the data structure. Must be larger
 *  than `n`. The allocator is required to reallocate at least, but not
 *  necessarily exactly this much elements.
 *  \param[in] sz Size in bytes of a single object in the underlying array.
 *  \param[in,out] user Private data passed through from the algorithm. In
 *  this case it is left unused.
 *
 *  \return Returns the size of the new array on success
 *
 *  #### Example: using a custom allocator to reallocate aligned arrays
 *
 *  ~~~{.c}
 *  static inline size_t aligned_reallocator(void *restrict *p, size_t n,
 *                                           size_t new_n, size_t sz,
 *                                           void *user)
 *  {
 *          size_t align = *(size_t *)user;
 *          void *q;
 *
 *          assert(n < RBIT(n, 0));
 *          q = tie_aligned_malloc(align, max(n * 2, new_n), sz);
 *          if (q) {
 *                  tie_free(*p);
 *          }
 *          *p = q;
 *          return n * 2;
 *  }
 *  // ...
 *  {
 *          size_t align = alignof(*mat);
 *          simd_algorithm(n, mat, aligned_reallocator, &align);
 *  }
 *  ~~~
 *
 *  \remark It is possible for the `sz` parameter to be zero; see
 *  #auxiliary_realloc_void().
 *
 *  \sa #auxiliary_realloc()
 */
static inline size_t auxiliary_reallocator(void **restrict p,
                                           size_t n,
                                           size_t new_n,
                                           size_t sz,
                                           UNUSED void *user)
{
        assert(!mul_overflow(log2size, n, (size_t)3));
        *p = tie_realloc(*p, max(new_n, n * 3 / 2), sz);
        return n * 3 / 2;
}

#endif
