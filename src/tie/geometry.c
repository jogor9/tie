#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "algo.h"
#include "array.h"
#include "attrib.h"
#include "base_array.h"
#include "bit.h"
#include "functional.h"
#include "geometry.h"
#include "heap.h"
#include "math.h"
#include "numeric_array.h"

sort_by_decl(vec2d,
             sort_vec2d_on_x,
             static inline,
             p,
             q,
             compare(vec_x(*p), vec_x(*q)),
             void *,
             user);

sort_by_decl(vec2d,
             sort_vec2d_on_y,
             static inline,
             p,
             q,
             compare(vec_y(*p), vec_y(*q)),
             void *,
             user);

typedef enum {
        PTVT_START,
        PTVT_END,
        PTVT_MERGE,
        PTVT_SPLIT,
        PTVT_UP,
        PTVT_DOWN
} PTVertexType;

PURE_FUNC static inline PTVertexType pt_get_vertex_type(
        const vec2d *v,
        size_t n,
        const vec2d polygon[static n])
{
        const vec2d *p, *q;
        vec2d pv, vq;

        p = circular_pred(v, polygon, polygon + n);
        q = circular_succ(v, polygon, polygon + n);

        if (compare(vec_y(*p), vec_y(*v)) == compare(vec_y(*v), vec_y(*q))) {
                if (compare(vec_y(*p), vec_y(*v)) > 0)
                        return PTVT_UP;
                return PTVT_DOWN;
        }

        pv = *v;
        sub_vec2d(&pv, p);

        vq = *q;
        sub_vec2d(&vq, v);

        if (vec_y(*p) > vec_y(*v)) { // end or merge vertex
                if (cross_vec2d(&pv, &vq) < 0)
                        return PTVT_MERGE;
                return PTVT_END;
        }

        if (cross_vec2d(&pv, &vq) < 0)
                return PTVT_SPLIT;

        return PTVT_START;
}

vec2d *convex_hull_sorted(size_t n,
                          const vec2d points[static restrict n],
                          vec2d out[static restrict n])
{
        const vec2d *r;
        vec2d *result = out, pq, qr;

        assert(n > 2);

        traverse(r, points, points + 2) {
                *result++ = *r;
        }
#define compute_next_r                                                         \
        pq = result[-1];                                                       \
        sub_vec2d(&pq, &result[-2]);                                           \
        qr = *r;                                                               \
        sub_vec2d(&qr, &result[-1]);                                           \
        while (cross_vec2d(&pq, &qr) < 0) {                                    \
                --result;                                                      \
                if (result - out == 1)                                         \
                        break;                                                 \
                pq = result[-1];                                               \
                sub_vec2d(&pq, &result[-2]);                                   \
                qr = *r;                                                       \
                sub_vec2d(&qr, &result[-1]);                                   \
        }                                                                      \
        *result++ = *r

        traverse(r, points + 2, points + n) {
                compute_next_r;
        }
        rtraverse(r, points + 1, points + n - 1) {
                compute_next_r;
        }

        return result;
}

vec2d *convex_hull(size_t n,
                   vec2d points[static restrict n],
                   vec2d out[static restrict n])
{
        sort_vec2d_on_x(n, points, NULL);
        return convex_hull_sorted(n, points, out);
}

void polygon_triangulate_convex(
        size_t n,
        const vec2d polygon[static restrict n],
        vec2d out[static restrict(2 * (n - 2) + 3 * (n % 2))])
{
        const vec2d *center = polygon, *p;

        traverse_step(p, 2, polygon + 1, polygon + n - 2) {
                *out++ = p[0];
                *out++ = *center;
                *out++ = p[1];
                *out++ = p[2];
        }

        if (is_odd(n)) {
                *out++ = polygon[n - 2];
                *out++ = *center;
                *out++ = polygon[n - 1];
        }
}

typedef const vec2d *pt_vecp;
heap_decl(pt_vecp,
          pt,
          static inline UNUSED,
          p,
          q,
          reverse_compare(vec_y(**p), vec_y(**q)) == 0
                  ? compare(vec_x(**p), vec_x(**q))
                  : reverse_compare(vec_y(**p), vec_y(**q)),
          void *,
          user);

typedef struct {
        pt_vecp begin;
        pt_vecp end;
} PTDiagonal;

static inline void pt_search_delete(pt_vecp p,
                                    pt_vecp **search,
                                    pt_vecp *search_end)
{
        pt_vecp *q, t;

        find(&p, q, *search, search_end);
        swap(*q, *search_end, t);
        *search += 1;
}

static inline void pt_search_insert(pt_vecp v, pt_vecp **search)
{
        *search -= 1;
        **search = v;
}

PURE_FUNC static inline double pt_solve_lin(const vec2d *restrict p0,
                                            const vec2d *restrict p1,
                                            const vec2d *restrict v)
{
        const double double_min = -0x1.p52;
        double x;

        if (compare(vec_y(*p0), vec_y(*v)) != compare(vec_y(*v), vec_y(*p1)))
                return double_min;

        if (fabs(vec_y(*p1) - vec_y(*p0)) < 0x1.p-52)
                return double_min;

        x = (vec_y(*v) - vec_y(*p1)) * (vec_x(*p1) - vec_x(*p0))
          / (vec_y(*p1) - vec_y(*p0));

        if (x >= vec_x(*v))
                return double_min;

        return x;
}

PURE_FUNC static inline pt_vecp pt_search(const vec2d *v,
                                          size_t n,
                                          const vec2d polygon[static n],
                                          const pt_vecp *search,
                                          const pt_vecp *search_end)
{
        const pt_vecp *out, *p;

        maximum_by(
                out,
                p,
                compare(pt_solve_lin(*out,
                                     circular_succ(*out, polygon, polygon + n),
                                     v),
                        pt_solve_lin(*p,
                                     circular_succ(*p, polygon, polygon + n),
                                     v)),
                search,
                search_end);

        return *out;
}

static inline void pt_diagonal_insert(pt_vecp begin,
                                      pt_vecp end,
                                      PTDiagonal **diagonals_end)
{
        (*diagonals_end)->begin = begin;
        (*diagonals_end)->end = end;
        *diagonals_end += 1;
}

static void pt_divide(uint32_t n,
                      vec2d polygon[static restrict n],
                      uint32_t m,
                      const PTDiagonal diagonals[static restrict n],
                      uint32_t out[static restrict(3 * (n - 2))])
{
}

static void polygon_triangulate_monotone(
        uint32_t n,
        const vec2d polygon[static restrict n],
        uint32_t out[static restrict(3 * (n - 2))],
        const vec2d *stack[static restrict(2 * n - 4)],
        const vec2d *ysorted[static restrict n])
{
}

void polygon_triangulate(uint32_t n,
                         const vec2d polygon[static restrict n],
                         uint32_t out[static restrict(3 * (n - 2))],
                         const vec2d *heap[static restrict n],
                         const vec2d *helper[static restrict n],
                         PTDiagonal diagonals[static restrict n])
{
        pt_vecp p, v, *i, *search = heap + n, *search_end = search, s;
        PTDiagonal *diagonals_end = diagonals, *d;
        PTVertexType vtype;
        size_t heapsz = n;

        traverse(i, heap, heap + heapsz)
                *i = polygon + (i - heap);
        pt_heap_build(heapsz, heap, NULL);

        traverse(i, heap, heap + n) {
                v = heap[0];
                pt_heap_pop(&heapsz, heap, NULL);
                p = circular_pred(v, polygon, polygon + n);

                // e-1 = vq
                // e = pv
                vtype = pt_get_vertex_type(v, n, polygon);
                if (vtype == PTVT_END || vtype == PTVT_MERGE
                    || vtype == PTVT_UP) {
                        if (pt_get_vertex_type(helper[v - polygon], n, polygon)
                            == PTVT_MERGE) {
                                pt_diagonal_insert(v,
                                                   helper[v - polygon],
                                                   &diagonals_end);
                        }
                        pt_search_delete(v, &search, search_end);
                }
                if (vtype == PTVT_SPLIT || vtype == PTVT_MERGE
                    || vtype == PTVT_DOWN) {
                        s = pt_search(v, n, polygon, search, search_end);
                        if (vtype == PTVT_SPLIT
                            || pt_get_vertex_type(
                                       helper[s - polygon], n, polygon)
                                       == PTVT_MERGE) {
                                pt_diagonal_insert(helper[s - polygon],
                                                   v,
                                                   &diagonals_end);
                        }
                        helper[s - polygon] = v;
                }
                if (vtype == PTVT_START || vtype == PTVT_SPLIT
                    || vtype == PTVT_UP) {
                        helper[p - polygon] = v;
                        pt_search_insert(p, &search);
                }
        }

        traverse(d, diagonals, diagonals_end) {
        }
}

PURE_FUNC double polygon_signed_area(size_t n,
                                     const vec2d polygon[static restrict n])
{
        double area;
        const vec2d *p;

        assert(n > 0);

        reduce(area,
               p,
               area + vec_x(p[0]) * vec_y(p[1]) - vec_x(p[1]) * vec_y(p[0]),
               0,
               polygon,
               polygon + n - 1);
        area += vec_x(p[n - 1]) * vec_y(p[0]) - vec_x(p[0]) * vec_y(p[n - 1]);

        return area * 0.5;
}

void de_casteljau(double t,
                  size_t n,
                  vec2d bezier[static restrict n],
                  vec2d left[restrict n])
{
        vec2d *p, *q;
        vec2d temp;

        for (q = bezier + n - 1; q >= bezier; --q) {
                if (left) {
                        *left++ = *bezier;
                }
                for (p = bezier; p < q; ++p) {
                        // p[0] = p[0] + t * (p[1] - p[0])
                        temp = p[1];
                        sub_vec2d(&temp, p);
                        scale_vec2d(&temp, t);
                        add_vec2d(p, &temp);
                }
        }
}

PURE_FUNC bool colinear(size_t n,
                        const vec2d points[static restrict n],
                        double error)
{
        assert(error >= 0);

        const double epsilon = 1.0 / (1 << 25);
        const vec2d *p, *q;
        vec2d diff;
        double sine, cosine;

        if (n <= 2) {
                return true;
        }

        // This algorithm works by choosing two random points and rotating
        // all of the points so that the two points lie on the X axis.
        // The absolute value of the Y component of any of the points will be
        // less than error if the points are colinear, and larger otherwise.

        p = points;
        q = points + 1;

        // we can't choose a point near equal p, since
        // we wouldn't be able to normalize their difference
        // if all points turn out equal we exit early to avoid
        // any floating point exceptions
        do {
                diff = *q;
                sub_vec2d(&diff, p);
                ++q;
        } while (q < points + n && fabs(diff.v[0]) < epsilon
                 && fabs(diff.v[1]) < epsilon);
        if (q >= points + n) {
                return true;
        }

        normalize_vec2d(&diff);

        cosine = vec_x(diff);
        sine = vec_y(diff);

        traverse(q, q, points + n) {
                diff = *q;
                sub_vec2d(&diff, p);
                if (fabs(cosine * vec_y(diff) - sine * vec_x(diff)) > error) {
                        return false;
                }
        }

        return true;
}

vec2d *bezier_discretize(size_t n,
                         const vec2d bezier[static restrict n],
                         size_t *restrict pout_sz,
                         vec2d *restrict *restrict pout,
                         size_t *restrict paux_sz,
                         vec2d *restrict *restrict paux,
                         double error,
                         Reallocator *reallocator,
                         void *user)
{
        vec2d *aux = *paux, *out = *pout;
        vec2d *result = out, *last = aux, *end = aux + n;
        size_t out_sz = *pout_sz, aux_sz = *paux_sz;

        assert(n > 0);
        assert(out_sz > 0);
        assert(aux_sz >= n);
        assert(error >= 0);

        memcpy(aux, bezier, n * sizeof(*bezier));
        *result++ = bezier[0];

        do {
                if (colinear(n, last, error)) {
                        if ((size_t)(result - out) == out_sz
                            && !auxiliary_realloc(reallocator,
                                                  &out_sz,
                                                  &out,
                                                  pout_sz,
                                                  pout,
                                                  out_sz + 1,
                                                  user)) {
                                return NULL;
                        }
                        *result++ = end[-1];
                        last -= n;
                        end -= n;
                        continue;
                }
                if (end - aux + n >= aux_sz
                    && !auxiliary_realloc(reallocator,
                                          &aux_sz,
                                          &aux,
                                          paux_sz,
                                          paux,
                                          end - aux + n,
                                          user)) {
                        return NULL;
                }
                de_casteljau(0.5, n, last, end);
                last += n;
                end += n;
        } while (end != aux);

        return result;
}

void furthest_points_apart(const vec2d **restrict out1,
                           const vec2d **restrict out2,
                           size_t n,
                           const vec2d points[static n])
{
        assert(n >= 2);

        const vec2d *p, *q;
        vec2d diffv;
        double max_diff = 0.0, diff;

        for_each_pair(p, q, points, points + n) {
                diffv = *p;
                sub_vec2d(&diffv, q);
                diff = sqrmag_vec2d(&diffv);
                if (diff > max_diff) {
                        max_diff = diff;
                        *out1 = p;
                        *out2 = q;
                }
        }
}
