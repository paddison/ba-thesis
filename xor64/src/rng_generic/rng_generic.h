#ifndef RNG_GENERIC_H
#define RNG_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "rng_generic_generators.h"

/**
 * Represents the current state of the rng, e.g. the current random number
 */
typedef struct Xor64RngGeneric Xor64RngGeneric;

Xor64RngGeneric* xor64_rng_generic_init(Xor64RngGeneric* rng);
Xor64RngGeneric* xor64_rng_generic_copy(Xor64RngGeneric* dest, const Xor64RngGeneric* source);
Xor64RngGeneric* xor64_rng_generic_add(Xor64RngGeneric* lhs, const Xor64RngGeneric* rhs);
uint64_t xor64_rng_generic_gen64(Xor64RngGeneric* rng); 
void xor64_rng_generic_next_state(Xor64RngGeneric* rng);
Xor64RngGeneric* xor64_rng_generic_seed(Xor64RngGeneric* rng, uint64_t seed); 

#ifdef __cplusplus
}
#endif

#endif
