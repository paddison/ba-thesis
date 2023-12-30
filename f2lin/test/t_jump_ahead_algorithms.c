#define TEST
#include <stdio.h>
#include "minunit.h"
#include "config.h"
#include "jump_ahead.h"
#include "rng_generic/rng_generic.h"

int tests_run = 0;

static void do_n_steps(size_t n, F2LinRngGeneric* rng) {
    for (size_t i = 0; i < n; ++i) f2lin_rng_generic_gen64(rng); 
}

static int test_jump(size_t jump_size, F2LinConfig* c) {
    int ret;
    F2LinRngGeneric* jump = f2lin_rng_generic_init();
    F2LinRngGeneric* iter = f2lin_rng_generic_init();
    uint64_t actual, expected;

    F2LinJump* params = f2lin_jump_ahead_init(jump_size, c);
    do_n_steps(jump_size, iter);
    f2lin_jump_ahead_jump(params, jump);
    actual = f2lin_rng_generic_gen64(jump); 
    expected = f2lin_rng_generic_gen64(iter);

    printf("actual: %llu\t expected: %llu\n", actual, expected);

    f2lin_rng_generic_destroy(jump);
    f2lin_rng_generic_destroy(iter);
    return actual == expected;
}

static char* test_algorithm(int q, enum F2LinJumpAlgorithm algorithm) {
    F2LinConfig c = { .q = q, .algorithm = algorithm };
    mu_assert("Wrong result with jump_size 1", test_jump(1, &c));
    mu_assert("Wrong result with jump_size 10", test_jump(10, &c));
    mu_assert("Wrong result with jump_size 100", test_jump(100, &c));
    mu_assert("Wrong result with jump_size 1000", test_jump(1000, &c));
    mu_assert("Wrong result with jump_size 10000", test_jump(10000, &c));
    mu_assert("Wrong result with jump_size 100000", test_jump(100000, &c));
    mu_assert("Wrong result with jump_size 1000000", test_jump(1000000, &c));

    return 0;
}

static char* test_horner() {
    // test for different jump sizes
    F2LinConfig c = { .q = 4, .algorithm = HORNER };
    printf("Testing algorithm HORNER\n");
    return test_algorithm(4, HORNER);
}

static char* test_sliding_window() { 
    F2LinConfig c = { .q = 4, .algorithm = SLIDING_WINDOW };
    printf("Testing algorithm SLIDING_WINDOW\n");
    return test_algorithm(4, SLIDING_WINDOW);
}

static char* test_sliding_window_decomp() {
    F2LinConfig c = { .q = 4, .algorithm = SLIDING_WINDOW_DECOMP };
    printf("Testing algorithm SLIDING_WINDOW_DECOMP\n");
    return test_algorithm(4, SLIDING_WINDOW_DECOMP);
}

static char* all_tests() {
    mu_run_test(test_horner);
    mu_run_test(test_sliding_window);
    mu_run_test(test_sliding_window_decomp);

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
