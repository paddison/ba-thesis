#include <stdio.h>
#include "xor64.h"
#include "jump.h"
#include "rng_generic.h"

/* Header Implementations */
Xor64RngGeneric* xor64_init() {
    return xor64_rng_generic_init();
}

Xor64RngGeneric* xor64_init_seed(const uint64_t seed) {
    Xor64 xor64 = { 0 };

    return xor64_rng_generic_init_seed(seed);
}

Xor64Jump* xor64_prepare_jump(size_t jump_size, Xor64Config* cfg) {
    Xor64Jump* jump = malloc(sizeof(Xor64Jump));
    return xor64_jump_init(jump, jump_size, cfg);
}

void xor64_jump(Xor64RngGeneric* rng, Xor64Jump* jump) {
    xor64_jump_jump(jump, rng);
}

void xor64_clear_jump(Xor64Jump *jump) {
    if (jump) {
        xor64_jump_destroy(jump); 
        free(jump);
    }
}

void xor64_destroy(Xor64RngGeneric* rng) {
    if (rng) { 
        free(rng);
        rng = 0;
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
