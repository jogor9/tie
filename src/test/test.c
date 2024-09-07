#include <SDL2/SDL_timer.h>
#include <stdio.h>

#include "tie/geometry.h"
#include "tie/math.h"

#define MAP(macro, arg, ...) macro(arg) __VA_OPT__(MAP(macro, __VA_ARGS__))
#define PRIM_CAT(x, ...) x##__VA_ARGS__
#define CAT(x, ...) PRIM_CAT(x, __VA_ARGS__)

#define IF(cond) PRIM_CAT(IF_, cond)
#define IF_0(t, f) f
#define IF_1(t, f) t

#define NOT(x) PRIM_CAT(NOT_, x)
#define NOT_0 1
#define NOT_1 0

#define AND(a) PRIM_CAT(AND_, a)
#define AND_0(b) 0
#define AND_1(b) b

#define OR(a) PRIM_CAT(OR_, a)
#define OR_0(b) b
#define OR_1(b) 1

#define EAT(...)
#define EXPAND(...) __VA_ARGS__
#define WHEN(x) IF(x)(EXPAND, EAT)

#define XOR(a) CAT(XOR_, b)
#define XOR_0(b) b
#define XOR_1(b) NOT(b)

// function_to_test(
//      // apply to all values from 1 to 10
//      arg0 = all_from_to(1, 10, int),
//      // apply to ten random values from range 3 to 42069 using xorshift
//      arg1 = random_from_to(3, 42069, 10, xorshift, &xs_state),
//      // apply to all from a given array of values
//      arg2 = all_from_range(selected_values, 10),
//      // apply to 5 random values from a given array using xorshift
//      arg3 = random_from_range(selected_values, s, 10, 5, xorshift, &xs_state)
//      // apply 10 invocations of a generator function
//      arg4 = generate_n(generator, seed, 10)
//      // choose permutations from 2nd to 4th of a range of 5 values
//      arg5 = perms_from_to(temp, 2, 4, range, 5),
//      // choose 10 random permutations of an array of 20 values
//      arg6 = random_perms(temp, s, values, 20, 10, xorshift, &xs_state),
//      arg7 = perms_from_range(temp, values, 20, selected, 10,
//      // select all permutations of a range of size 3
//      arg7 = all_perms(temp, range, 3, xorshift, &xs_state),
//      arg8 = permutations_from_range(temp, range, 5, perm_values)
//      arg9 = generate_n
//

#define SZ (1 << 10)

int main(void)
{
        static vec2d line_strip[SZ];
        static vec2d stack[SZ];
        static const vec2d bezier[4] = {
                { .v = { 0.0, 0.0 } },
                { .v = { -0.25, 1.0 } },
                { .v = { 0.5, 0.125 } },
                { .v = { 1.0, 0.5, } },
        };
        vec2d *out = line_strip, *aux = stack, *out_end;
        size_t out_sz = SZ, aux_sz = SZ;
        Uint64 start, end;

        start = SDL_GetPerformanceCounter();
        out_end = bezier_discretize(4,
                                    bezier,
                                    &out_sz,
                                    &out,
                                    &aux_sz,
                                    &aux,
                                    1.0 / (1 << 15),
                                    NULL,
                                    NULL);
        end = SDL_GetPerformanceCounter();

        fprintf(stderr,
                "took %lfms, resulting in %ld points\n",
                (end - start) / ((double)SDL_GetPerformanceFrequency() / 1000),
                out_end - out);
        return 0;
}
