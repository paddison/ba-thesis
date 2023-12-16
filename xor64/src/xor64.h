#ifndef PXORSHIFT_H
#define PXORSHIFT_H

#include <stdlib.h>
#include "rng_generic/rng_generic.h"
#include "config.h"

/* Opaque pointer definition to hide implementation details */
typedef struct Xor64Jump Xor64Jump;

typedef struct Xor64 Xor64;

struct Xor64 {
    Xor64RngGeneric rng;
    Xor64Jump* jump;
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
Xor64 xor64_init();

/**
 * Initialize the state Random number generator with @param seed.
 * If @param xor_64 is 0, it will be allocated on the heap and returned to the caller.
 *
 */
Xor64 xor64_init_seed(const uint64_t seed);

/**
 * Initialize the jump polynomial for the random number generator to a jump size of 
 * @param jump_size. Can only be called after pxor64_init or pxor64_init_seed.
 *
 * This can also be used to change the jump size later, but ONLY after a call to 
 * xor64_clear_jump.
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
void xor64_prepare_jump(Xor64* xor64, const size_t jump_size, Xor64Config* cfg);

/**
 * Jump forward in the stream by @param jump steps.
 * Can only be called after pxor64_prepare_jump.
 */
void xor64_jump(Xor64* xor64);

/**
 * Clear the parameters for jumping ahead, freeing any space used by them. 
 * Only allowed to be called after xor64_prepare_jump.
 *
 * There are two scenarios when a user might call this function:
 * 1. When changing the jump size, before a new call to xor64_prepare_jump.
 * 2. When cleaning up the application, in order to avoid leaks.
 */
void xor64_clear_jump(Xor64* xor64);

/**
 * Generates the next unsigned 64 bit number in the stream.
 * '
 * Can only be used after a call to xor64_init()
 */ 
static inline uint64_t xor64_next_unsigned(Xor64* xor64) {
    return xor64_rng_generic_gen64(&xor64->rng);
}

/**
 * Generates the next signed 64 bit number in the stream.
 * '
 * Can only be used after a call to xor64_init()
 */ 
static inline int64_t xor64_next_signed(Xor64* xor64) {
    return (int64_t) xor64_rng_generic_gen64(&xor64->rng);
}

/**
 * Generates the next real number in the stream. The number will be in the range of
 * 0 (inclusive) to 1 (exclusive).
 * '
 * Can only be used after a call to xor64_init()
 */ 
static inline double xor64_next_double(Xor64* xor64) {
    uint64_t num = xor64_rng_generic_gen64(&xor64->rng);
    return (num >> 11) * (1.0/9007199254740992.0);
}

#endif
