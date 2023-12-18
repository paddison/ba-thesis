#ifndef XOR64_JUMP_H
#define XOR64_JUMP_H

#include <stdlib.h>

#include "config.h"

typedef struct Xor64Jump Xor64Jump;
typedef struct Xor64RngGeneric Xor64RngGeneric;
typedef struct GF2X GF2X;
typedef struct Xor64PolyDecomp Xor64PolyDecomp;

struct Xor64Jump {
    size_t q;
    enum JumpAlgorithm algorithm;
    Xor64RngGeneric** h;
    GF2X* jump_poly;
    Xor64PolyDecomp* decomp_poly;
};

Xor64Jump* xor64_jump_ahead_init(size_t jump_size, Xor64Config* c);
Xor64RngGeneric* xor64_jump_ahead_jump(Xor64Jump* jump_params, Xor64RngGeneric* rng);
void xor64_jump_ahead_destroy(Xor64Jump* jump_params);

#endif
