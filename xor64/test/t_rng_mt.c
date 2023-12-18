#include <stdio.h>
#include "minunit.h"
#include "config.h"
#include "jump.h"
#include "rng_generic/rng_generic.h"

int tests_run = 0;

static void do_n_steps(size_t n, Xor64RngGeneric* rng) {
    for (size_t i = 0; i < n; ++i) xor64_rng_generic_gen64(rng); 
}

static int compare_state(Xor64RngGeneric* iter, Xor64RngGeneric* jump) {
    return iter->mt.mt[iter->mt.mti] == jump->mt.mt[jump->mt.mti];
}

static int test_jump(size_t jump_size, Xor64Config* c) {
    int ret;
    Xor64RngGeneric jump;
    Xor64RngGeneric iter;

    xor64_rng_generic_init(&jump);
    xor64_rng_generic_init(&iter);

    Xor64Jump params = { 0 };
    xor64_jump_init(&params, jump_size, c);
    do_n_steps(jump_size, &iter);
    xor64_jump_jump(&params, &jump);

    ret = compare_state(&iter, &jump);

    return ret;
}

static char* test_algorithm(int q, enum JumpAlgorithm algorithm) {
    Xor64Config c = { .q = q, .algorithm = algorithm };
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
    Xor64Config c = { .q = 4, .algorithm = HORNER };
    printf("Testing algorithm HORNER");
    test_algorithm(4, SLIDING_WINDOW);
    return 0;
}

static char* test_sliding_window() { 
    Xor64Config c = { .q = 4, .algorithm = SLIDING_WINDOW };
    printf("Testing algorithm SLIDING_WINDOW");
    test_algorithm(4, SLIDING_WINDOW);
    return 0;
}

static char* test_sliding_window_decomp() {
    Xor64Config c = { .q = 4, .algorithm = SLIDING_WINDOW_DECOMP };
    printf("Testing algorithm SLIDING_WINDOW_DECOMP");
    test_algorithm(4, SLIDING_WINDOW_DECOMP);
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
