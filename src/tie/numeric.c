#include "attrib.h"
#include "numeric.h"

PURE_FUNC size_t max_factorial(size_t size_bytes)
{
#define MAX_FACTORIAL_SIZE 64
        static const unsigned char sizes[MAX_FACTORIAL_SIZE] = {
                5,  8,  10, 12, 14, 16, 18, 20, 22, 24, 25, 27, 29, 30, 32, 34,
                35, 37, 38, 40, 41, 43, 44, 46, 47, 48, 50, 51, 53, 54, 55, 57,
                58, 59, 61, 62, 64, 65, 66, 67, 69, 70, 71, 73, 74, 75, 77, 78,
                79, 80, 82, 83, 84, 85, 87, 88, 89, 90, 91, 93, 94, 95, 96, 98
        };

        assert(size_bytes > 0 && size_bytes <= MAX_FACTORIAL_SIZE);

        return sizes[size_bytes - 1];
}
