#include <stdio.h>
#include "xor64.h"
#include "jump.h"
#include "rng_generic/rng_generic.h"

/* Header Implementations */
Xor64 xor64_init() {
    Xor64 xor64 = { 0 };
    xor64.rng = calloc(1, sizeof(Xor64RngGeneric));
    xor64_rng_generic_init(xor64.rng);
    xor64.jump = 0;

    return xor64;
}

Xor64 xor64_init_seed(const uint64_t seed) {
    Xor64 xor64 = { 0 };

    xor64_rng_generic_seed(xor64.rng, seed);
    return xor64;
}

void xor64_prepare_jump(Xor64* xor64, size_t jump_size, Xor64Config* cfg) {
    if (xor64->jump) {
        fprintf(stderr, 
                "ERROR: Trying to call xor64_prepare_jump \
                with already initialized jump parameters.\n");
        return;
    }
    xor64->jump = malloc(sizeof(Xor64Jump));
    xor64_jump_init(xor64->jump, jump_size, cfg);
}

void xor64_jump(Xor64* xor64) {
    if (!xor64->jump) {
        fprintf(stderr, "ERROR: xor64_prepare_jump has to be called before jumping\n");
        return;
    }

    xor64_jump_jump(xor64->jump, xor64->rng);
}

void xor64_clear_jump(Xor64 *xor64) {
    xor64_jump_destroy(xor64->jump); 
    free(xor64->jump);
    xor64->jump = 0;
}

void xor64_destroy(Xor64 *xor64) {
    if (xor64->jump)  {
        xor64_jump_destroy(xor64->jump); 
        free(xor64->jump);
        xor64->jump = 0;
    }
    if (xor64->rng) { 
        free(xor64->rng);
        xor64->rng = 0;
    }
}


uint64_t xor64_next_unsigned(Xor64* xor64) {
    return xor64_rng_generic_gen64(xor64->rng);
}

int64_t xor64_next_signed(Xor64* xor64) {
    return (int64_t) xor64_rng_generic_gen64(xor64->rng);
}

double xor64_next_double(Xor64* xor64) {
    uint64_t num = xor64_rng_generic_gen64(xor64->rng);
    return (num >> 11) * (1.0/9007199254740992.0);
}
