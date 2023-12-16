#ifndef XOR64_JUMP_H
#define XOR64_JUMP_H

#include <stdlib.h>

#include "rng_generic/rng_generic.h"
#include "poly_decomp.h"
#include "gf2x_wrapper.h"
#include "config.h"

typedef struct Xor64Jump Xor64Jump;

struct Xor64Jump {
    size_t q;
    enum JumpAlgorithm algorithm;
    Xor64RngGeneric h[1 << Q_MAX];
    GF2X* jump_poly;
    Xor64PolyDecomp  decomp_poly;
};

Xor64Jump* xor64_jump_init(Xor64Jump* jump_params, size_t jump_size, Xor64Config* c);
Xor64RngGeneric* xor64_jump_jump(Xor64Jump* jump_params, Xor64RngGeneric* rng);
void xor64_jump_destroy(Xor64Jump* jump_params);

#endif
