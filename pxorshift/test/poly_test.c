#include <stdio.h>
#include "minunit.h"
#include "poly.h"
#include "xorshift64.h"

#define SEED 362436000ull

int tests_run = 0;

StateRng s_jump = (StateRng) { .s = SEED };
StateRng s_iter = (StateRng) { .s = SEED };
const StateRng orig = (StateRng) { .s = SEED };

static void do_n_steps(size_t n, StateRng* rng) {
    for (size_t i = 0; i < n; ++i) step(rng); 
}

static void reset_states() {
    state_rng_copy(&s_iter, &orig);
    state_rng_copy(&s_jump, &orig);
}

static int compare_states() {
    return s_jump.s == s_iter.s;
}

static int test_jump(size_t jump_size) {
    int ret;

    init_jump(jump_size);
    do_n_steps(jump_size, &s_iter);
    jump(&s_jump);

    ret = compare_states();
    reset_states();

    return ret;
}

static char* test_horner() {
    // test for different jump sizes
    set_jump_algorithm(HORNER);
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
    return 0;
}

static char* test_sliding_window_decomp() {
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
