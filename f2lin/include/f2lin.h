#ifndef PXORSHIFT_H
#define PXORSHIFT_H

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "config.h"

/* Opaque pointer definitions to hide implementation details */
typedef struct F2LinJump F2LinJump;
typedef struct F2LinRngGeneric F2LinRngGeneric;

/**
 * Initialize the Random number generator and return a pointer to it. 
 * The seed will be predefined.
 *
 * A generator created from this function must be destroyed with f2lin_rng_destroy()
 * at the end of the application.
 *
 */
F2LinRngGeneric* f2lin_rng_init();

/**
 * Initialize Random number generator with @param seed and return a pointer to it.
 *
 * A generator created from this function must be destroyed with f2lin_rng_destroy()
 * at the end of the application.
 *
 */
F2LinRngGeneric*  f2lin_rng_init_seed(const uint64_t seed);

/**
 * Initialize the jump parameters for the random number generator to a jump size of 
 * @param jump_size. 
 *
 * The returned pointer must be destroyed by a call to f2lin_jump_destroy().
 *
 * This can also be used to create several jumps for different jump sizes.
 *
 * It is not necessary though to call this several times with the same jump size.
 *
 * Optionally @param cfg can be used to configure the application. 
 *
 * @param cfg is a struct containing two fields: 
 * -jump_algorithm: enum JumpAlgorithm
 *  The algorithm used for jumping. Possible values are:
 *      1. HORNER, 2. SLIDING_WINDOW, 3. SLIDING_WINDOW_DECOMP. 
 * -q: size_t
 * Only has an effect if jump_algorithm is SLIDING_WINDOW and SLIDING_WINDOW_DECOMP
 * This sets the size of the decomposition polynomials, when decomposing the jump polynomial.
 * Depending on the jump polynomial, different sizes for q can influence the performance.
 * has to be in the range of 1 - 10.
 *
 * See http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/ARTICLES/jumpf2-printed.pdf for a
 * detailed description of what these parameters are.
 *
 * @param cfg can also be set to 0. In this, the default values will be used. These are:
 *      jump_algorithm = SLIDING_WINDOW_DECOMP
 *      q = 3
 */
F2LinJump* f2lin_jump_init(const size_t jump_size, F2LinConfig* cfg);

/**
 * Jump @param rng forward in the stream, according to the parameters set in @param jump.
 */
void f2lin_jump(F2LinRngGeneric* rng, F2LinJump* jump);

/**
 * Destroy the parameters for jumping ahead, freeing any space used by them. 
 */
void f2lin_jump_destroy(F2LinJump* jump);

/**
 * Destroys the random number generator, freeing all memory used by it.
 */
void f2lin_rng_destroy(F2LinRngGeneric* rng);

/**
 * Generates the next unsigned 64 bit number in the stream.
 * '
 * Can only be used after a call to f2lin_init()
 */ 
uint64_t f2lin_next_unsigned(F2LinRngGeneric* rng);

/**
 * Generates the next signed 64 bit number in the stream.
 * '
 * Can only be used after a call to f2lin_init()
 */ 
int64_t f2lin_next_signed(F2LinRngGeneric* rng);

/**
 * Generates the next real number in the stream. The number will be in the range of
 * 0 (inclusive) to 1 (exclusive).
 * '
 * Can only be used after a call to f2lin_init()
 */ 
double f2lin_next_double(F2LinRngGeneric* rng);

#endif
