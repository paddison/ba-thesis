#include <stdio.h>
#include "xor64.h"

/* Header Implementations */
Xor64* xor64_init(Xor64* xor64) {
    if (!xor64) xor64 = malloc(sizeof(Xor64));

    xor64_rng_generic_init(&xor64->rng);
    xor64->jump = 0;
    return xor64;
}


Xor64* xor64_init_seed(Xor64* xor64, const uint64_t seed) {
    if (!xor64) xor64 = malloc(sizeof(Xor64));

    xor64_rng_generic_seed(&xor64->rng, seed);
    xor64->jump = 0;
    return xor64;
}

void xor64_prepare_jump(Xor64* xor64, size_t jump_size, Config* cfg) {
    // free the jump
    if (xor64->jump) { 
        xor64_jump_destroy(xor64->jump);
        free(xor64->jump);
    }
    xor64->jump = malloc(sizeof(Xor64Jump));
    xor64_jump_init(xor64->jump, &xor64->rng, jump_size, cfg);
}

void xor64_jump(Xor64* xor64) {
    if (!xor64->jump) {
        fprintf(stderr, "ERROR: xor64_prepare_jump has to be called before jumping\n");
        return;
    }

    xor64_jump_jump(xor64->jump, &xor64->rng);
}
