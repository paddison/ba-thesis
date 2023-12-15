#ifndef XOR64_RNG_GENERIC_64_H
#define XOR64_RNG_GENERIC_64_H

#ifdef __cpluplus
extern "C" {
#endif

#include <stdlib.h>

//#include "xor64_rng_generic.h"

#define XOR64_RNG_STATE_SIZE 64

typedef struct Xor64RngGeneric Xor64RngGeneric;

struct Xor64RngGeneric {
    uint64_t state;
};

#ifdef __cpluplus
}
#endif
#endif 
