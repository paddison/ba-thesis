#ifndef XOR64_JUMP_H
#define XOR64_JUMP_H

#include <stdlib.h>

#include "config.h"
#include <stdint.h>
#include <inttypes.h>

typedef struct F2LinRngGeneric F2LinRngGeneric;
typedef struct GF2X GF2X;
typedef struct F2LinPolyDecomp F2LinPolyDecomp;

typedef struct F2LinJumpSW F2LinJumpSW;
struct F2LinJumpSW {
    int q;
    F2LinRngGeneric** y;
    GF2X* jp;
};

typedef struct F2LinJumpSWD F2LinJumpSWD;
struct F2LinJumpSWD {
    int q;
    F2LinRngGeneric** y;
    F2LinPolyDecomp* pd;
};

union F2LinJumpPoly {
    GF2X* horner;
    F2LinJumpSW sw;
    F2LinJumpSWD swd;
};

typedef struct F2LinJump F2LinJump;
struct F2LinJump {
    enum F2LinJumpAlgorithm algorithm;
    union F2LinJumpPoly jp;
};

F2LinJump* f2lin_jump_ahead_init(const size_t jump_size, F2LinConfig* c);
F2LinRngGeneric* f2lin_jump_ahead_jump(F2LinJump* jump_params, F2LinRngGeneric* rng);
void f2lin_jump_ahead_destroy(F2LinJump* jump_params);

#endif
