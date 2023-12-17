#include <stdio.h>
#include <stdlib.h>
#include "xor64.h"

int main (void) {
    Xor64 seq = xor64_init();
    Xor64 rng = xor64_init();

    xor64_prepare_jump(&rng, 1000, 0);
    xor64_jump(&rng);

    for (size_t i = 0; i < 1000; ++i) xor64_next_unsigned(&seq);

    printf("num: %llu\n", xor64_next_unsigned(&rng));
    printf("num: %llu\n", xor64_next_unsigned(&seq));

    xor64_clear_jump(&rng);

    xor64_prepare_jump(&rng, 100000, 0);
    xor64_jump(&rng);

    for (size_t i = 0; i < 100000; ++i) xor64_next_unsigned(&seq);

    printf("num: %llu\n", xor64_next_unsigned(&rng));
    printf("num: %llu\n", xor64_next_unsigned(&seq));

    xor64_destroy(&rng);
    xor64_destroy(&seq);

    return EXIT_SUCCESS;
}
