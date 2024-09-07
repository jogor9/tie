#ifndef TIE_FUNCTIONAL_H
#define TIE_FUNCTIONAL_H

#define swap(a, b, t) ((t) = (a), (a) = (b), (b) = (t))
#define compare(a, b) (((a) > (b)) - ((b) > (a)))
#define reverse_compare(a, b) compare(b, a)
#define equality(a, b) ((a) == (b))
#define inequality(a, b) ((a) != (b))
// on(equality, abs, *p, *q)
#define on(bin, map, a, b) bin(map(a), map(b))

#endif
