#ifndef RNG_GENERIC_XOR64_H
#define RNG_GENERIC_XOR64_H

#include "rng_generic.h"

#define STATE_SIZE 64

struct RngGeneric {
    uint64_t s;
};

#endif
