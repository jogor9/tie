#ifndef TIE_ARRAY_ALGO_H
#define TIE_ARRAY_ALGO_H

#include <stddef.h>

#include "bit.h"
#include "functional.h"

#define array_size(arr) (sizeof(arr) / sizeof(*(arr)))
#define traverse_array(p, arr) for (p = arr; p < arr + array_size(arr); ++p)

#define range_bytes(begin, end) (((end) - (begin)) * sizeof(*(begin)))

#define traverse_step(p, step, begin, end)                                     \
        for ((p) = (begin); (p) < (end); (p) += (step))
#define traverse(p, begin, end) traverse_step(p, 1, begin, end)

#define rtraverse_step(p, step, begin, end)                                    \
        for ((p) = (end) - 1; (p) >= (begin); (p) -= (step))
#define rtraverse(p, begin, end) rtraverse_step(p, 1, begin, end)

#define zip_step(p, q, pbegin, pend, qbegin, qend, step)                       \
        for ((p) = (pbegin), (q) = (qbegin); (p) < (pend) && (q) < (qend);     \
             (p) += (step), (q) += (step))
#define zip(p, q, pbegin, pend, qbegin, qend)                                  \
        zip_step(p, q, pbegin, pend, qbegin, qend, 1)
#define zip_array_step(p, q, parr, qarr, step)                                 \
        zip_step(p,                                                            \
                 q,                                                            \
                 parr,                                                         \
                 (parr) + array_size(parr),                                    \
                 qarr,                                                         \
                 (qarr) + array_size(qarr),                                    \
                 step)
#define zip_array(p, q, parr, qarr) zip_array_step(p, q, parr, qarr, 1)

#define for_each_pair(p, q, begin, end)                                        \
        traverse(q, begin, end)                                                \
                traverse(p, begin, q)
#define for_each_triple(p, q, r, begin, end)                                   \
        traverse(r, begin, end)                                                \
                for_each_pair(p, q, begin, r)
#define for_each_of_cartesian(p, q, begin, end, begin2, end2)                  \
        traverse(q, begin2, end2)                                              \
                traverse(p, begin, end)

#define filter(p, expr, begin, end)                                            \
        for ((p) = (begin); (p) < (end); ++(p))                                \
                if (expr)
#define filter_array(p, expr, arr) filter(p, expr, arr, (arr) + array_size(arr))

#define take(n, p, begin, end)                                                 \
        for ((p) = (begin); (begin) < (end) && (begin) < (begin) + (n); ++(p))
#define take_array(n, p, arr) take(n, p, arr, (arr) + array_size(arr))

#define group_by(p, q, rbegin, rend, expr, begin, end)                         \
        for ((p) = (begin), (q) = (begin) + 1, (rbegin) = (p), (rend) = (p);   \
             (q) <= (end);                                                     \
             ++(p), ++(q))                                                     \
                if (((q) == (end)                                              \
                     && (ptr_swap(rbegin, rend), (rend) = (q), 1))             \
                    || ((q) != (end) && !(expr)                                \
                        && (ptr_swap(rbegin, rend), (rend) = (q), 1)))
#define group(p, q, rbegin, rend, begin, end)                                  \
        group_by(p, q, rbegin, rend, equality(*(p), *(q)), begin, end)

// expr should return a negative value if l < r
// zero if they're considered equal
// and a positive value if l > r
#define reduce(acc, p, expr, z, begin, end)                                    \
        for ((acc) = (z), (p) = (begin); (p) < (end); ++(p))                   \
                (acc) = (expr);                                                \
        (void)0
#define sum(acc, p, begin, end) reduce(acc, p, (acc) + *(p), 0, begin, end)
#define product(acc, p, begin, end) reduce(acc, p, (acc) * *(p), 1, begin, end)
#define average(acc, p, begin, end)                                            \
        sum(acc, p, begin, end);                                               \
        (acc) /= (end) - (begin);                                              \
        (void)0
#define faverage(acc, p, begin, end)                                           \
        reduce(acc, p, (acc) + *(p) / ((end) - (begin)), 0, begin, end)
#define maximum_by(out, p, expr, begin, end)                                   \
        reduce(out, p, (expr) < 0 ? (p) : (out), begin, (begin) + 1, end)
#define maximum(out, p, begin, end)                                            \
        maximum_by(out, p, compare(*(out), *(p)), begin, end)
#define minimum_by(out, p, expr, begin, end)                                   \
        reduce(out, p, (expr) < 0 ? (out) : (p), begin, (begin) + 1, end)
#define minimum(out, p, expr, begin, end)                                      \
        minimum_by(out, p, compare(*(out), *(p)), begin, end)

#define find_by(p, expr, begin, end)                                           \
        filter(p, expr, begin, end)                                            \
                break;                                                         \
        (void)0
#define find(q, p, begin, end) find_by(p, *(p) == *(q), begin, end)

#define find_ordered_by(p, expr, begin, end)                                   \
        filter(p, (expr) <= 0, begin, end)                                     \
                break;                                                         \
        (void)0
#define find_ordered(q, p, begin, end) find_by(p, compare(*q, *p), begin, end)
#define insertion_sort(p, q, begin, end, cmp, temp)                            \
        traverse(p, (begin) + 1, end) {                                        \
                for ((q) = (p); (q) > (begin) && cmp((q)[0], (q)[-1]) < 0;     \
                     --(q)) {                                                  \
                        swap((q)[0], (q)[-1], temp);                           \
                }                                                              \
        }                                                                      \
        (void)0
#define sort_by_decl(type, funname, cmp, usertype, user)                       \
        static inline void funname(size_t n,                                   \
                                   type arr[static restrict n],                \
                                   usertype user)                              \
        {                                                                      \
                type *r;                                                       \
                type *q;                                                       \
                type *p;                                                       \
                type *v = arr;                                                 \
                type temp;                                                     \
                type pivot;                                                    \
                                                                               \
                do {                                                           \
                        /* insertion sort if small enough */                   \
                        if (n <= 16) {                                         \
                                insertion_sort(p, q, v, v + n, cmp, temp);     \
                                return;                                        \
                        }                                                      \
                                                                               \
                        /* find pivot */                                       \
                        p = v;                                                 \
                        q = v + n / 2;                                         \
                        r = v + n - 1;                                         \
                        if (cmp(*p, *q) > 0) /* v[0] > v[n/2] */               \
                                swap(*p, *q, temp);                            \
                        if (cmp(*q, *r) > 0) /* v[n/2] > v[n-1] */             \
                                swap(*q, *r, temp);                            \
                        if (cmp(*p, *q) > 0) /* v[0] > v[n/2] */               \
                                swap(*p, *q, temp);                            \
                        pivot = *q;                                            \
                                                                               \
                        /* partition */                                        \
                        do {                                                   \
                                while (cmp(*p, pivot) < 0)                     \
                                        ++p;                                   \
                                while (cmp(*r, pivot) > 0)                     \
                                        --r;                                   \
                                if (p >= r)                                    \
                                        break;                                 \
                                swap(*p, *r, temp);                            \
                        } while (1);                                           \
                                                                               \
                        /* select smaller range to recurse on */               \
                        /* otherwise tail call */                              \
                        if (r > q) {                                           \
                                funname(v + n - (r + 1), r + 1, user);         \
                                /* tail call */                                \
                                n = r - v;                                     \
                        } else {                                               \
                                funname(r - v, v, user);                       \
                                /* tail call */                                \
                                n -= (r + 1) - v;                              \
                                v = r + 1;                                     \
                        }                                                      \
                } while (1);                                                   \
        }                                                                      \
        static inline void funname(size_t n,                                   \
                                   type v[static restrict n],                  \
                                   usertype user)
#define sort_decl(type, funname)                                               \
        sort_by_decl(type, funname, compare, void *user)
#define bsearch_by_decl(type, funname, attribs, cmp, usertype, user)           \
        static inline const type *funname(const type *q,                       \
                                          size_t n,                            \
                                          const type arr[static restrict n],   \
                                          usertype user)                       \
        {                                                                      \
                const type *v = arr;                                           \
                const type *p;                                                 \
                int c;                                                         \
                                                                               \
                do {                                                           \
                        if (n == 0)                                            \
                                break;                                         \
                                                                               \
                        p = v + n / 2;                                         \
                        c = cmp(*p, *q);                                       \
                        if (c == 0)                                            \
                                break;                                         \
                                                                               \
                        if (c < 0) {                                           \
                                n = n - (p + 1 - v);                           \
                                v = p + 1;                                     \
                        } else {                                               \
                                n = p - v;                                     \
                        }                                                      \
                } while (1);                                                   \
                                                                               \
                return p;                                                      \
        }                                                                      \
        static inline const type *funname(const type *q,                       \
                                          size_t n,                            \
                                          const type v[static restrict n])

#define circular_pred(p, begin, end) ((p) == (begin) ? (end) - 1 : (p) - 1)
#define circular_succ(p, begin, end) ((p) + 1 == (end) ? (begin) : (p) + 1)

#endif
