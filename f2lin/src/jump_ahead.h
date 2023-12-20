#ifndef XOR64_JUMP_H
#define XOR64_JUMP_H

#include <stdlib.h>

#include "config.h"
#include <stdint.h>
#include <inttypes.h>

typedef struct F2LinJump F2LinJump;
typedef struct F2LinRngGeneric F2LinRngGeneric;
typedef struct GF2X GF2X;
typedef struct F2LinPolyDecomp F2LinPolyDecomp;

struct F2LinJump {
    size_t q;
    enum F2LinJumpAlgorithm algorithm;
    F2LinRngGeneric** h;
    GF2X* jump_poly;
    F2LinPolyDecomp* decomp_poly;
};

F2LinJump* f2lin_jump_ahead_init(const size_t jump_size, F2LinConfig* c);
F2LinRngGeneric* f2lin_jump_ahead_jump(F2LinJump* jump_params, F2LinRngGeneric* rng);
void f2lin_jump_ahead_destroy(F2LinJump* jump_params);

#endif
