#include <stdio.h>
#include <stdlib.h>
#include "xor64.h"
#include "config.h"

int main (void) {
    Xor64RngGeneric* seq = xor64_rng_init();
    Xor64RngGeneric* rng = xor64_rng_init();
    Xor64Config cfg = { .q = 4, .algorithm = HORNER };

    Xor64Jump* jump = xor64_jump_init(100, &cfg);
    xor64_jump(rng, jump);

    for (size_t i = 0; i < 100; ++i) xor64_next_unsigned(seq);

    printf("num: %llu\n", xor64_next_unsigned(rng));
    printf("num: %llu\n", xor64_next_unsigned(seq));

    xor64_jump_destroy(jump);

    jump = xor64_jump_init(100000, &cfg);
    xor64_jump(rng, jump);

    for (size_t i = 0; i < 100000; ++i) xor64_next_unsigned(seq);

    printf("num: %llu\n", xor64_next_unsigned(rng));
    printf("num: %llu\n", xor64_next_unsigned(seq));

    xor64_jump_destroy(jump);

    xor64_rng_destroy(rng);
    xor64_rng_destroy(seq);

    return EXIT_SUCCESS;
}
