#include <stdio.h>
#include "f2lin.h"
#include "jump_ahead.h"
#include "rng_generic/rng_generic.h"

/* Header Implementations */
F2LinRngGeneric* f2lin_rng_init() {
    return f2lin_rng_generic_init();
}

F2LinRngGeneric* f2lin_rng_init_seed(const uint64_t seed) {
    return f2lin_rng_generic_init_seed(seed);
}

F2LinJump* f2lin_jump_init(size_t jump_size, F2LinConfig* cfg) {
    return f2lin_jump_ahead_init(jump_size, cfg);
}

void f2lin_jump(F2LinRngGeneric* rng, F2LinJump* jump) {
    if (!rng || !jump) {
        fprintf(stderr, "Trying to call f2lin_jump with uninitialized pointers\n");
    }
    f2lin_jump_ahead_jump(jump, rng);
}

void f2lin_jump_destroy(F2LinJump *jump) {
    if (jump) {
        f2lin_jump_ahead_destroy(jump); 
    }
}

void f2lin_rng_destroy(F2LinRngGeneric* rng) {
    if (rng) { 
        f2lin_rng_generic_destroy(rng);
    }
}

uint64_t f2lin_next_unsigned(F2LinRngGeneric* rng) {
    if (!rng) { 
        fprintf(stderr, "Trying to generate unsigned number with uninitialized rng\n");
        return -1;
    }
    return f2lin_rng_generic_gen64(rng);
}

int64_t f2lin_next_signed(F2LinRngGeneric* rng) {
    if (!rng) { 
        fprintf(stderr, "Trying to generate signed number with uninitialized rng\n");
        return -1;
    }
    return (int64_t) f2lin_rng_generic_gen64(rng);
}

double f2lin_next_double(F2LinRngGeneric* rng) {
    if (!rng)  {
        fprintf(stderr, "Trying to generate double number with uninitialized rng\n");
        return -1;
    }
    uint64_t num = f2lin_rng_generic_gen64(rng);
    return (num >> 11) * (1.0/9007199254740992.0);
}
