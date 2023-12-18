#ifndef RNG_GENERIC_H
#define RNG_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/**
 * Represents the current state of the rng, e.g. the current random number
 */
typedef struct Xor64RngGeneric Xor64RngGeneric;

Xor64RngGeneric* xor64_rng_generic_init_zero();
Xor64RngGeneric* xor64_rng_generic_init();
Xor64RngGeneric* xor64_rng_generic_copy(Xor64RngGeneric* dest, const Xor64RngGeneric* source);
Xor64RngGeneric* xor64_rng_generic_add(Xor64RngGeneric* lhs, const Xor64RngGeneric* rhs);
uint64_t xor64_rng_generic_gen64(Xor64RngGeneric* rng); 
void xor64_rng_generic_next_state(Xor64RngGeneric* rng);
Xor64RngGeneric* xor64_rng_generic_init_seed(uint64_t seed); 
void xor64_rng_generic_destroy(Xor64RngGeneric* rng);
long xor64_rng_generic_state_size();

// ask christian if this is good style or not
void xor64_rng_generic_gen_n_numbers(Xor64RngGeneric* rng, size_t N, uint64_t buf[N]);


#ifdef TEST
int xor64_rng_generic_compare_state(Xor64RngGeneric* lhs, Xor64RngGeneric* rhs);
#endif

#ifndef CALC_MIN_POLY
char* xor64_rng_generic_min_poly();
#endif


#ifdef __cplusplus
}
#endif

#endif
