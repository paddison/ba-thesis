#ifndef PXORSHIFT_H
#define PXORSHIFT_H

// function for floating point, 64bit int, initializing, etc.
#include <stdlib.h>
#include "polynomials.hpp"
#include "rng_generic.h"
#include "xor64_poly.h"

struct Xor64 {
    RngGeneric state;
    Xor64Poly jp;
    Config cfg;
};

/**
 * Initialize the the state Random number generator. The seed will be predefined.
 * If @param xor_64 is 0, xor_64 will be heap allocated and returned to the caller.
 */
Xor64* pxor64_init(Xor64* xor_64);

/**
 * Initialize the state Random number generator with @param seed.
 * If @param xor_64 is 0, it will be allocated on the heap and returned to the caller.
 */
Xor64* pxor64_init_seed(Xor64* xor_64, uint64_t seed);

/**
 * Initialize the jump polynomial for the random number generator to a jump size of 
 * @param jump. Can only be called after pxor64_init or pxor64_init_seed.
 */
void pxor64_prepare_jump(Xor64* xor_64, size_t jump);

/**
 * Jump forward in the stream by @param jump steps.
 * Can only be called after pxor64_prepare_jump.
 */
void pxor64_jump(Xor64* xor_64);

/**
 * Configure @param xor_64 with @param cfg. This influences the way in which the jump 
 * will be calculated. 
 *
 * @param cfg is a struct containing two fields: 
 * -jump_algorithm: The algorithm used for jumping. Possible values are:
 * 1. HORNER, 2. SLIDING_WINDOW, 3. SLIDING_WINDOW_DECOMP. 
 * -q: Only has an effect if jump_algorithm is SLIDING_WINDOW and SLIDING_WINDOW_DECOMP
 * This sets the size of the decomposition polynomials, when decomposing the jump polynomial
 * depending on the size of the jump polynomials, different sizes for q can influence the runtime.
 * q also has to be in the range of 1 - 10.
 *
 * @param cfg can also be set to 0. In this, the default values will be used. These are:
 * jump_algorithm = SLIDING_WINDOW_DECOMP
 * q = 3
 *
 * This has to be done before calling pxor64_prepare_jump.
 */
Xor64* pxor_64_configure(Xor64* xor_64, Config* cfg);

uint64_t pxor64_next_unsigned64();

int64_t pxor64_next_signed64();

double pxor64_next_();

#endif
