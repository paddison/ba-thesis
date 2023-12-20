#ifndef RNG_GENERIC_H
#define RNG_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

/**
 * Represents the current state of the rng, e.g. the current random number
 */
typedef struct F2LinRngGeneric F2LinRngGeneric;

F2LinRngGeneric* f2lin_rng_generic_init_zero();
F2LinRngGeneric* f2lin_rng_generic_init();
F2LinRngGeneric* f2lin_rng_generic_copy(F2LinRngGeneric* dest, const F2LinRngGeneric* source);
F2LinRngGeneric* f2lin_rng_generic_add(F2LinRngGeneric* lhs, const F2LinRngGeneric* rhs);
uint64_t f2lin_rng_generic_gen64(F2LinRngGeneric* rng); 
uint64_t f2lin_rng_generic_next_state(F2LinRngGeneric* rng);
F2LinRngGeneric* f2lin_rng_generic_init_seed(uint64_t seed); 
void f2lin_rng_generic_destroy(F2LinRngGeneric* rng);
long f2lin_rng_generic_state_size();

// ask christian if this is good style or not
void f2lin_rng_generic_gen_n_numbers(F2LinRngGeneric* rng, size_t N, uint64_t buf[N]);


#ifdef TEST
int f2lin_rng_generic_compare_state(F2LinRngGeneric* lhs, F2LinRngGeneric* rhs);
#endif

#ifndef CALC_MIN_POLY
char* f2lin_rng_generic_min_poly();
#endif


#ifdef __cplusplus
}
#endif

#endif
