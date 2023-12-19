#include <stdio.h>
#include <stdlib.h>
#include "f2lin.h"
#include "config.h"

int main (void) {
    F2LinRngGeneric* seq = f2lin_rng_init();
    F2LinRngGeneric* rng = f2lin_rng_init();
    F2LinConfig cfg = { .q = 4, .algorithm = SLIDING_WINDOW_DECOMP };

    F2LinJump* jump = f2lin_jump_init(100, &cfg);
    f2lin_jump(rng, jump);

    for (size_t i = 0; i < 100; ++i) f2lin_next_unsigned(seq);

    printf("jump: %llu\n", f2lin_next_unsigned(rng));
    printf("iter: %llu\n", f2lin_next_unsigned(seq));

    f2lin_jump_destroy(jump);

    jump = f2lin_jump_init(1000, &cfg);
    f2lin_jump(rng, jump);

    for (size_t i = 0; i < 1000; ++i) f2lin_next_unsigned(seq);

    printf("jump: %llu\n", f2lin_next_unsigned(rng));
    printf("iter: %llu\n", f2lin_next_unsigned(seq));

    f2lin_jump_destroy(jump);

    f2lin_rng_destroy(rng);
    f2lin_rng_destroy(seq);

    return EXIT_SUCCESS;
}
