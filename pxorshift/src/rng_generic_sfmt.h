#ifndef RNG_GENERIC_SFMT_H
#define RNG_GENERIC_SFMT_H

#include "rng_generic.h"

#ifdef __cplusplus

extern "C" {

#endif

#include "../lib/sfmt/SFMT.h"

#define STATE_SIZE 19937

struct RngGeneric {
    sfmt_t sfmt;
};

#ifdef __cplusplus

}

#endif
#endif
