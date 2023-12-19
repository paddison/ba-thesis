#include <stdio.h>
#include "xor64.h"
#include "jump_ahead.h"
#include "rng_generic/rng_generic.h"

/* Header Implementations */
Xor64RngGeneric* xor64_rng_init() {
    return xor64_rng_generic_init();
}

Xor64RngGeneric* xor64_rng_init_seed(const uint64_t seed) {
    Xor64 xor64 = { 0 };

    return xor64_rng_generic_init_seed(seed);
}

Xor64Jump* xor64_jump_init(size_t jump_size, Xor64Config* cfg) {
    return xor64_jump_ahead_init(jump_size, cfg);
}

void xor64_jump(Xor64RngGeneric* rng, Xor64Jump* jump) {
    xor64_jump_ahead_jump(jump, rng);
}

void xor64_jump_destroy(Xor64Jump *jump) {
    if (jump) {
        xor64_jump_ahead_destroy(jump); 
    }
}

void xor64_rng_destroy(Xor64RngGeneric* rng) {
    if (rng) { 
        xor64_rng_generic_destroy(rng);
    }
}

uint64_t xor64_next_unsigned(Xor64RngGeneric* rng) {
    return xor64_rng_generic_gen64(rng);
}

int64_t xor64_next_signed(Xor64RngGeneric* rng) {
    return (int64_t) xor64_rng_generic_gen64(rng);
}

double xor64_next_double(Xor64RngGeneric* rng) {
    uint64_t num = xor64_rng_generic_gen64(rng);
    return (num >> 11) * (1.0/9007199254740992.0);
}
