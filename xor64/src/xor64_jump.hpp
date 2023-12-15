#ifndef XOR64_JUMP_H
#define XOR64_JUMP_H

#include "NTL/GF2X.h"
#ifdef __cplusplus 


#include "xor64_poly_decomp.hpp"
extern "C" {

#endif
#include <stdlib.h>

#include "xor64_rng_generic.h"

#define Q_MAX 10

/**
 * Which algorithm to use for jumping ahead in the random number stream
 * Default is SLIDING_WINDOW.
 */
enum JumpAlgorithm {
    DEFAULT = 0, HORNER = 1, SLIDING_WINDOW = 2, SLIDING_WINDOW_DECOMP = 3, 
};

typedef struct Xor64Jump Xor64Jump;

struct Xor64Jump {
    size_t q;
    enum JumpAlgorithm algorithm;
    Xor64RngGeneric h[1 << Q_MAX];
    NTL::GF2X jump_poly;
    Xor64PolyDecomp decomp_poly;
};

/**
 * Used for configuring the application.
 * q is the degree of the decomposition polynomials when using the sliding window method
 */
typedef struct Config Config;

struct Config {
    enum JumpAlgorithm algorithm;
    size_t q;
};

Xor64Jump* xor64_jump_init(Xor64Jump* jump_params, Xor64RngGeneric* rng, size_t jump_size, Config* c);
Xor64RngGeneric* xor64_jump_jump(Xor64Jump* jump_params, Xor64RngGeneric* rng);
void xor64_jump_destroy(Xor64Jump* jump_params);

#ifdef __cplusplus 

}

#endif

#endif
