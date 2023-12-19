#ifndef PXORSHIFT_H
#define PXORSHIFT_H

#include <stdlib.h>
#include "config.h"

/* Opaque pointer definitions to hide implementation details */
typedef struct F2LinJump F2LinJump;
typedef struct F2LinRngGeneric F2LinRngGeneric;

typedef struct F2Lin F2Lin;

struct F2Lin {
    F2LinRngGeneric* rng;
    F2LinJump* jump;
};

/**
 * Initialize the the state Random number generator. The seed will be predefined.
 * If @param xor_64 is 0, xor_64 will be heap allocated and returned to the caller.
 *
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
 * jump_algorithm = SLIDING_WINDOW_DECOMP
 * q = 3
 */
F2LinRngGeneric* f2lin_rng_init();

/**
 * Initialize the state Random number generator with @param seed.
 * If @param xor_64 is 0, it will be allocated on the heap and returned to the caller.
 *
 */
F2LinRngGeneric*  f2lin_rng_init_seed(const uint64_t seed);

/**
 * Initialize the jump polynomial for the random number generator to a jump size of 
 * @param jump_size. Can only be called after pf2lin_init or pf2lin_init_seed.
 *
 * This can also be used to change the jump size later, but ONLY after a call to 
 * f2lin_clear_jump.
 *
 * Optionally @param cfg can be used to configure the application. 
 *
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
 * jump_algorithm = SLIDING_WINDOW_DECOMP
 * q = 3
 */
F2LinJump* f2lin_jump_init(const size_t jump_size, F2LinConfig* cfg);

/**
 * Jump forward in the stream by @param jump steps.
 * Can only be called after pf2lin_prepare_jump.
 */
void f2lin_jump(F2LinRngGeneric* rng, F2LinJump* jump);

/**
 * Clear the parameters for jumping ahead, freeing any space used by them. 
 * Only allowed to be called after f2lin_prepare_jump.
 *
 * There are two scenarios when a user might call this function:
 * 1. When changing the jump size, before a new call to f2lin_prepare_jump.
 * 2. When cleaning up the application, in order to avoid leaks.
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
