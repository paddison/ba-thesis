#include <stdio.h>
#include <stdlib.h>
#include "xor64.h"
#include "config.h"

int main (void) {
    Xor64RngGeneric* seq = xor64_init();
    Xor64RngGeneric* rng = xor64_init();
    Xor64Config cfg = { .q = 4, .algorithm = SLIDING_WINDOW_DECOMP };

    Xor64Jump* jump = xor64_prepare_jump(100, &cfg);
    xor64_jump(rng, jump);

    for (size_t i = 0; i < 100; ++i) xor64_next_unsigned(seq);

    printf("num: %llu\n", xor64_next_unsigned(rng));
    printf("num: %llu\n", xor64_next_unsigned(seq));

    xor64_clear_jump(jump);

    jump = xor64_prepare_jump(100000, &cfg);
    xor64_jump(rng, jump);

    for (size_t i = 0; i < 100000; ++i) xor64_next_unsigned(seq);

    printf("num: %llu\n", xor64_next_unsigned(rng));
    printf("num: %llu\n", xor64_next_unsigned(seq));

    xor64_clear_jump(jump);

    xor64_destroy(rng);
    xor64_destroy(seq);

    return EXIT_SUCCESS;
}
