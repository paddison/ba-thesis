#define TEST

#include <stdio.h>
#include "minunit.h"
#include "config.h"
#include "jump_ahead.h"
#include "rng_generic/rng_generic.h"

int tests_run = 0;

inline static void do_n_steps(size_t n, F2LinRngGeneric* rng) {
    for (size_t i = 0; i < n; ++i) f2lin_rng_generic_gen64(rng); 
}

static char* test_first_100k() {
    size_t START = 0;
    size_t MAX = 635;
    F2LinConfig c = { .q = 4, .algorithm = SLIDING_WINDOW_DECOMP };
    F2LinRngGeneric* iter= f2lin_rng_generic_init();
    do_n_steps(START, iter);
    
    for (size_t i = START + 1; i < MAX; ++i) {
        F2LinRngGeneric* jump = f2lin_rng_generic_init();
        F2LinJump* params = f2lin_jump_ahead_init(i, &c);
        f2lin_jump_ahead_jump(params, jump);
        f2lin_rng_generic_gen64(iter);

        if (!f2lin_rng_generic_compare_state(jump, iter)) {
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