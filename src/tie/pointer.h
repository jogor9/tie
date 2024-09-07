#ifndef TIE_POINTER_H
#define TIE_POINTER_H

#include <stddef.h>
#include <stdint.h>

#define declval(type) (*(type *)NULL)

// (n - a mod n) mod n
#define ptr_align(ptr, align)                                                  \
        ((void *)((uintptr_t)(ptr)                                             \
                  + ((align) - (uintptr_t)(ptr) % (align)) % (align)))

#endif
