#ifndef RNG_GENERIC_H
#define RNG_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
/**
 * Represents the current state of the rng, e.g. the current random number
 */
typedef struct RngGeneric RngGeneric;

RngGeneric* rng_generic_init(RngGeneric* rng);
RngGeneric* rng_generic_copy(RngGeneric* dest, const RngGeneric* source);
RngGeneric* rng_generic_add(RngGeneric* lhs, const RngGeneric* rhs);
uint64_t rng_generic_step(RngGeneric* rng); 
void rng_generic_next_state(RngGeneric* rng);
RngGeneric* rng_generic_seed(RngGeneric* rng, void* seed); 

#ifdef __cplusplus
}
#endif

#endif
