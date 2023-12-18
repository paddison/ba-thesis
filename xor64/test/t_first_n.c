#define TEST

#include <stdio.h>
#include "minunit.h"
#include "config.h"
#include "jump_ahead.h"
#include "rng_generic.h"

int tests_run = 0;

static void do_n_steps(size_t n, Xor64RngGeneric* rng) {
    for (size_t i = 0; i < n; ++i) xor64_rng_generic_gen64(rng); 
}


static int test_jump(size_t jump_size, Xor64Config* c) {
    int ret;
    Xor64RngGeneric* jump = xor64_rng_generic_init();
    Xor64RngGeneric* iter = xor64_rng_generic_init();


    Xor64Jump params = { 0 };
    xor64_jump_ahead_init(&params, jump_size, c);
    do_n_steps(jump_size, iter);
    xor64_jump_ahead_jump(&params, jump);

    ret = xor64_rng_generic_compare_state(iter, jump);

    return ret;
}

static char* test_first_100k() {
    size_t START = 0;
    size_t MAX = 1000;
    Xor64Config c = { .q = 4, .algorithm = SLIDING_WINDOW_DECOMP };
    Xor64RngGeneric* iter= xor64_rng_generic_init();
    for (size_t i = 0; i < START; ++i) xor64_rng_generic_next_state(iter);
    
    for (size_t i = START + 1; i < MAX; ++i) {
        Xor64RngGeneric* jump = xor64_rng_generic_init();
        Xor64Jump params = { 0 };

        xor64_jump_ahead_init(&params, i, &c);
        xor64_jump_ahead_jump(&params, jump);
        xor64_rng_generic_gen64(iter);

        if (!xor64_rng_generic_compare_state(jump, iter)) {
            printf("jump: %zu\n", i);
        }
    }

    return 0;
}




static char* all_tests() {
    mu_run_test(test_first_100k);

    return 0;
}

int main(void) {
    char* result = all_tests(); 

    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }

    printf("Tests run: %d\n", tests_run);

    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}