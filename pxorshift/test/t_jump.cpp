#include <stdio.h>
#include <stdlib.h>
#include "../src/minunit.h"
#include "../src/polynomials.hpp"

int tests_run = 0;

static char* test_jump_1() {
    RngGeneric step, jump;
    uint64_t expected, actual;
    Config c = { .q = 3, .algorithm = SLIDING_WINDOW_DECOMP };

    rng_generic_init(&step);
    rng_generic_init(&jump);

    polynomials_init(&c);
    polynomials_init_jump(1);

    rng_generic_step(&step);
    expected = rng_generic_step(&step);
    polynomials_jump(&jump);
    actual = rng_generic_step(&jump);

    mu_assert("Jump of 1 gives different result to step",
            actual == expected);

    return 0;
}

static char* test_jump_first_1000000() {
    RngGeneric step = { 0 };
    uint64_t expected, actual;
    Config c = { .q = 3, .algorithm = HORNER };

    rng_generic_init(&step);
    polynomials_init(&c);

    for (size_t i = 0; i < 1000000; ++i) {
        printf("%zu\n", i);
        RngGeneric jump = { 0 };
        rng_generic_init(&jump);
        polynomials_init_jump(i);
        polynomials_jump(&jump);
        actual = rng_generic_step(&jump);
        expected = rng_generic_step(&step);

        mu_assert("Wrong result during the first 1000000 jumps",
                actual == expected);
    }

    return 0;
}

static char* test_jump_19936() {
    RngGeneric step = { 0 };
    RngGeneric jump = { 0 };

    uint64_t expected, actual;
    Config c = { .q = 3, .algorithm = HORNER };
    rng_generic_init(&step);
    rng_generic_init(&jump);

    polynomials_init(&c);
    polynomials_init_jump(623);
    printf("jumping...\n");
    polynomials_jump(&jump);

    for (size_t i = 0; i < 623; ++i) rng_generic_step(&step);

    mu_assert("Wrong result for jumpsize of 20000", 
              rng_generic_step(&step) == rng_generic_step(&jump));
    return 0;
}

static char* all_tests() {
    //mu_run_test(test_jump_first_1000000);
    mu_run_test(test_jump_19936);

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
