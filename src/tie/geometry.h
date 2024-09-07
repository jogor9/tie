/*! \file geometry_algo.h
 *  \brief Geometry algorithms
 *
 *  This file contains various algorithms used to transform and compute
 *  properties of geometric objects. See documentation for algo.h for
 *  information on how to interface with TIE algorithms.
 */
#ifndef TIE_GEOMETRY_ALGOS_H
#define TIE_GEOMETRY_ALGOS_H

#include "algo.h"
#include "attrib.h"
#include "math.h"

/*! \brief Computes the signed area of a simple polygon using the shoelace
 *  formula.
 *
 *  The sign of the area reveals the polygon's orientation. A polygon is
 *  oriented positively, if by going through each of the points in order, we
 *  would always have the interior of the polygon on our right side; for
 *  negatively oriented polygons, we'd always have the interior on our left
 *  side. In other words, we make a counter-clockwise rotation when going
 *  through a positively oriented polygon's points, and a clockwise rotation
 *  when going through a negatively oriented polygon.
 *
 *  The input polygon must be simple, i.e. not self-intersecting.
 *
 *  \param[in] n The amount of points in the polygon.
 *  \param[in] polygon The points describing the polygon.
 *
 *  \return The signed area of the polygon.
 */
PURE_FUNC extern double polygon_signed_area(
        size_t n,
        const vec2d polygon[static restrict n]);

/*! \brief Splits a bezier curve defined by control points [first, first + n).
 *
 *  Leaves the right split of the curve in [first, first + n).
 *  If left is not NULL, also leaves the left split of the curve there.
 *  Runs in \f$O(n^2)\f$ time, requires \f$O(n)\f$ space.
 *
 *  \param[in] t The split point. Must be in the [0, 1] range.
 *  \param[in] n The amount of control points that the curve has, or in other
 *  words the order of the curve plus one. May be zero, in which case this
 *  function is a no-op.
 *  \param[in,out] bezier The input curve; will be consumed and replaced by
 *  the right split of the curve after the algorithm's run.
 *  \param[out] left May be NULL. If not NULL, points to an array of size
 *  `n` that will be filled with control points describing the left split
 *  of the curve.
 *
 *  \sa bezier_discretize()
 */
extern void de_casteljau(double t,
                         size_t n,
                         vec2d bezier[static restrict n],
                         vec2d left[restrict n]);

/*! \brief Test if the given points are colinear up to a given error.
 *
 *  Runs in \f$O(n)\f$. For n <= 2 always returns true.
 *
 *  \param[in] n The size of the `points` array.
 *  \param[in] points The points to process.
 *  \param[in] error \f$+-\f$ the amount of derivation a point can be from
 *  a straight line to be considered a part of the line. Must be nonnegative.
 *
 *  \return `true` if the points are colinear, `false` otherwise
 */
extern PURE_FUNC bool colinear(size_t n,
                               const vec2d points[static restrict n],
                               double error);

/*! \brief Computes a line strip that approximately describes a bezier curve.
 *
 *  This algorithm works by subdividing the bezier curve until the subcurves
 *  degenerate to a (near) straight line. The `error` parameter controls the
 *  maximal tolerable deviation from a straight line. The `paux` array serves
 *  as a stack for the curve subdivision. The `pout` array is where the line
 *  strip is output during the runtime of the algorithm. Because this algorithm
 *  is based on a heuristic, it may need to allocate way more memory than
 *  expected in the worst case; for dealing with memory reallocations see the
 *  algo.h documentation for details.
 *
 *  \param[in] n Amount of control points in the bezier curve. Must be positive.
 *  \param[in] bezier Control points describing the bezier curve.
 *  \param[in,out] pout_sz Initial size of the output array. Modified on
 *  reallocations. Initial value must be at least 1.
 *  \param[out] pout Output array to which the line strip will be written to.
 *  The pointer will be modified on reallocations.
 *  \param[in,out] paux_sz Initial size of the auxiliary array. Modified on
 *  reallocations. Initial value must be at least `n`.
 *  \param paux Auxiliary array for internal use in the algorithm.
 *  The pointer will be modified on reallocations.
 *  \param[in] error Passed to colinear(), see its documentation for details.
 *  \param[in] reallocator Reallocator in case the provided arrays are not
 *  sufficient. It should handle both arrays independently. May be NULL,
 *  in which case the function fails when the initial arrays become full.
 *  \param[in,out] user Private data to pass to the reallocator.
 *
 *  \return Returns a pointer one past the last point of the line strip, or
 *  NULL on allocation failure.
 *
 *  \sa colinear()
 */
vec2d *bezier_discretize(size_t n,
                         const vec2d bezier[static restrict n],
                         size_t *restrict pout_sz,
                         vec2d *restrict *restrict pout,
                         size_t *restrict paux_sz,
                         vec2d *restrict *restrict paux,
                         double error,
                         Reallocator *reallocator,
                         void *user);

/*! \brief Puts the two furthest points in out1 and out2.
 *
 *  Currently this algorithm takes n * (n - 1) / 2
 *  vector subtractions, multiplications, summations and scalar comparisons.
 *
 *  \param[out] out1 First point of the pair. Guaranteed to be ordered before
 *  `out2` (if that's important).
 *  \param[out] out2 Second point of the pair.
 *  \param[in] n Amount of points to process.
 *  \param[in] points The points to process.
 */
void furthest_points_apart(const vec2d **restrict out1,
                           const vec2d **restrict out2,
                           size_t n,
                           const vec2d points[static n]);
#endif
