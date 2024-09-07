#ifndef TIE_HEAP_H
#define TIE_HEAP_H

#include "base_array.h"
#include "numeric.h"

#define heap_decl(type, name, cmp, usertype, user)                             \
        static inline void name##_heap_sift_up(size_t n,                       \
                                               type heap[static restrict n],   \
                                               usertype user)                  \
        {                                                                      \
                type temp, *p, *q;                                             \
                q = heap + n - 1;                                              \
                p = heap + (q - heap - 1) / 2;                                 \
                                                                               \
                while (p > heap && cmp(*p, *q) > 0) {                          \
                        swap(*p, *q, temp);                                    \
                        q = p;                                                 \
                        p = heap + (q - heap - 1) / 2;                         \
                }                                                              \
        }                                                                      \
        static inline void name##_heap_push(const type *restrict item,         \
                                            size_t *restrict n,                \
                                            type heap[restrict],               \
                                            usertype user)                     \
        {                                                                      \
                heap[*n] = *item;                                              \
                *n += 1;                                                       \
                name##_heap_sift_up(*n, heap, user);                           \
        }                                                                      \
        static inline void name##_heap_sift_down(size_t n,                     \
                                                 type heap[static restrict n], \
                                                 usertype user)                \
        {                                                                      \
                type temp, *p, *q, *r;                                         \
                                                                               \
                p = heap;                                                      \
                                                                               \
                while (p < heap + n) {                                         \
                        q = heap + (r - heap) * 2 + 1;                         \
                        r = heap + (r - heap) * 2 + 2;                         \
                        q = min_by(cmp(*q, *r), q, r);                         \
                        if (cmp(*p, *q) <= 0)                                  \
                                break;                                         \
                        swap(*p, *q, temp);                                    \
                        p = q;                                                 \
                }                                                              \
        }                                                                      \
        static inline void name##_heap_pop(size_t *restrict n,                 \
                                           type heap[restrict],                \
                                           usertype user)                      \
        {                                                                      \
                *n -= 1;                                                       \
                heap[0] = heap[*n];                                            \
                name##_heap_sift_down(*n, heap, user);                         \
        }                                                                      \
        static inline void name##_heap_swap(const type *restrict item,         \
                                            size_t n,                          \
                                            type heap[static restrict n],      \
                                            usertype user)                     \
        {                                                                      \
                const type *p, *q;                                             \
                                                                               \
                p = item;                                                      \
                q = heap;                                                      \
                if (cmp(item, heap) <= 0) {                                    \
                        heap[0] = *item;                                       \
                        return;                                                \
                }                                                              \
                heap[0] = *item;                                               \
                name##_heap_sift_down(n, heap, user);                          \
        }                                                                      \
        static inline void name##_heap_build(size_t n,                         \
                                             type heap[static restrict n],     \
                                             usertype user)                    \
        {                                                                      \
                size_t i;                                                      \
                for (i = n / 2; i > 0; --i)                                    \
                        name##_heap_sift_down(i, heap, user);                  \
        }                                                                      \
        static inline void name##_heap_build(size_t n,                         \
                                             type heap[static restrict n],     \
                                             usertype user)
#endif
