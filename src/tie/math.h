#ifndef TIE_MATH_H
#define TIE_MATH_H

#include <assert.h>
#include <math.h>
#include <stdalign.h>
#include <stdbool.h>

#include "base_array.h"

typedef struct {
        alignas(16) double v[2];
} vec2d;

#define make_vec2d(x, y)                                                       \
        {                                                                      \
                .v = { x, y }                                                  \
        }
#define vec_x(vec) ((vec).v[0])
#define vec_y(vec) ((vec).v[1])
#define vec_z(vec) ((vec).v[2])
#define vec_w(vec) ((vec).v[3])

typedef union {
        vec2d cols[2];
} mat2d;

#define scalev_decl(vtype, name, op)                                           \
        static inline void name##_##vtype(vtype *restrict out, double s)       \
        {                                                                      \
                double *v;                                                     \
                traverse_array(v, out->v)                                      \
                        *v = *v op s;                                          \
        }                                                                      \
        static inline void name##_##vtype(vtype *restrict out, double s)
#define zipv_decl(vtype, name, op)                                             \
        static inline void name##_##vtype(vtype *restrict acc,                 \
                                          const vtype *restrict v)             \
        {                                                                      \
                double *l;                                                     \
                const double *r;                                               \
                zip_array(l, r, acc->v, v->v)                                  \
                        *l = *l op * r;                                        \
        }                                                                      \
        static inline void name##_##vtype(vtype *restrict acc,                 \
                                          const vtype *restrict v)
#define mapv_decl(vtype, name, p, expr)                                        \
        static inline void name##_##vtype(vtype *restrict v)                   \
        {                                                                      \
                double *p;                                                     \
                traverse_array(p, v->v)                                        \
                        *p = (expr);                                           \
        }                                                                      \
        static inline void name##_##vtype(vtype *restrict v)
#define reducev_decl(vtype, name, sumtype, zero, acc, p, expr)                 \
        PURE_FUNC static inline sumtype name##_##vtype(                        \
                const vtype *restrict v)                                       \
        {                                                                      \
                const double *p;                                               \
                sumtype acc = zero;                                            \
                traverse_array(p, v->v)                                        \
                        acc = (expr);                                          \
                return acc;                                                    \
        }                                                                      \
        PURE_FUNC static inline sumtype name##_##vtype(const vtype *restrict v)
#define zipreducev_decl(vtype, name, sumtype, zero, acc, p, q, expr)           \
        PURE_FUNC static inline sumtype name##_##vtype(                        \
                const vtype *restrict v, const vtype *restrict v2)             \
        {                                                                      \
                const double *p, *q;                                           \
                sumtype acc = zero;                                            \
                zip_array(p, q, v->v, v2->v)                                   \
                        acc = (expr);                                          \
                return acc;                                                    \
        }                                                                      \
        PURE_FUNC static inline sumtype name##_##vtype(                        \
                const vtype *restrict v, const vtype *restrict v2)
#define transformv_decl(rtype, vtype, mtype)                                   \
        PURE_FUNC static inline rtype transform_##mtype(                       \
                const vtype *restrict v, const mtype *restrict m)              \
        {                                                                      \
                const vtype *col;                                              \
                rtype result;                                                  \
                static_assert(                                                 \
                        array_size(v->v) == array_size(m->cols->v),            \
                        "Size of input vector must equal the number of rows"); \
                static_assert(array_size(result.v) == array_size(m->cols),     \
                              "Size of output vector must equal the number "   \
                              "of columns");                                   \
                traverse_array(col, m->cols)                                   \
                        result.v[col - m->cols] = dot_##vtype(v, col);         \
                return result;                                                 \
        }                                                                      \
        PURE_FUNC static inline rtype transform_##mtype(                       \
                const vtype *restrict v, const mtype *restrict m)
#define normv_decl(vtype)                                                      \
        static inline void normalize_##vtype(vtype *restrict v)                \
        {                                                                      \
                scale_##vtype(v, 1.0 / sqrt(sqrmag_##vtype(v)));               \
        }                                                                      \
        static inline void normalize_##vtype(vtype *restrict v)

scalev_decl(vec2d, scale, *);
zipv_decl(vec2d, add, +);
zipv_decl(vec2d, sub, -);
zipv_decl(vec2d, mul, *);
mapv_decl(vec2d, square, p, *p **p);
reducev_decl(vec2d, sum, double, 0, acc, p, acc + *p);
reducev_decl(vec2d, sqrmag, double, 0, acc, p, acc + *p * *p);
zipreducev_decl(vec2d, dot, double, 0, acc, p, q, acc + *p * *q);
transformv_decl(vec2d, vec2d, mat2d);
normv_decl(vec2d);

static inline double cross_vec2d(const vec2d *restrict a,
                                 const vec2d *restrict b)
{
        return vec_x(*a) * vec_y(*b) - vec_x(*b) * vec_y(*a);
}

#endif
