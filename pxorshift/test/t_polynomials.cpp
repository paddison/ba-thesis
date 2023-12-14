#include <stdio.h>
#include "../src/minunit.h"
#include "../src/polynomials.hpp"
#include "../src/rng_generic_xor64.h"

#define SEED 362436000ull

int tests_run = 0;

RngGeneric ss_jump = (RngGeneric) { .s = SEED };
RngGeneric s_iter = (RngGeneric) { .s = SEED };
const RngGeneric orig = (RngGeneric) { .s = SEED };

static void do_n_steps(size_t n, RngGeneric* rng) {
    for (size_t i = 0; i < n; ++i) rng_generic_step(rng); 
}

static void reset_states() {
    rng_generic_copy(&s_iter, &orig);
    rng_generic_copy(&ss_jump, &orig);
}

static int compare_states() {
    return ss_jump.s == s_iter.s;
}

static int test_jump(size_t jump_size) {
    int ret;

    polynomials_init_jump(jump_size);
    do_n_steps(jump_size, &s_iter);
    polynomials_jump(&ss_jump);

    ret = compare_states();
    reset_states();

    return ret;
}

static char* test_horner() {
    // test for different jump sizes
    Config c = { .q = 4, .algorithm = HORNER };
    polynomials_init(&c);
    mu_assert("Wrong result with horner at jump_size 1", test_jump(1));
    mu_assert("Wrong result with horner at jump_size 10", test_jump(10));
    mu_assert("Wrong result with horner at jump_size 100", test_jump(100));
    mu_assert("Wrong result with horner at jump_size 1000", test_jump(1000));
    mu_assert("Wrong result with horner at jump_size 10000", test_jump(10000));
    mu_assert("Wrong result with horner at jump_size 100000", test_jump(100000));
    mu_assert("Wrong result with horner at jump_size 1000000", test_jump(1000000));

    return 0;
}

static char* test_sliding_window() { 
    Config c = { .q = 4, .algorithm = SLIDING_WINDOW };
    polynomials_init(&c);
    mu_assert("Wrong result with horner at jump_size 1", test_jump(1));
    mu_assert("Wrong result with horner at jump_size 10", test_jump(10));
    mu_assert("Wrong result with horner at jump_size 100", test_jump(100));
    mu_assert("Wrong result with horner at jump_size 1000", test_jump(1000));
    mu_assert("Wrong result with horner at jump_size 10000", test_jump(10000));
    mu_assert("Wrong result with horner at jump_size 100000", test_jump(100000));
    mu_assert("Wrong result with horner at jump_size 1000000", test_jump(1000000));
    return 0;
}

static char* test_sliding_window_decomp() {
    Config c = { .q = 4, .algorithm = SLIDING_WINDOW_DECOMP };
    polynomials_init(&c);

    mu_assert("Wrong result with horner at jump_size 1", test_jump(1));
    mu_assert("Wrong result with horner at jump_size 10", test_jump(10));
    mu_assert("Wrong result with horner at jump_size 100", test_jump(100));
    mu_assert("Wrong result with horner at jump_size 1000", test_jump(1000));
    mu_assert("Wrong result with horner at jump_size 10000", test_jump(10000));
    mu_assert("Wrong result with horner at jump_size 100000", test_jump(100000));
    mu_assert("Wrong result with horner at jump_size 1000000", test_jump(1000000));
    return 0;
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
