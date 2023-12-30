#include <stdio.h>
#include <stdlib.h>
#include "minunit.h"
#include "gf2x_wrapper.h"
#include "rng_generic/rng_generic.h"

int tests_run = 0;

static F2LinRngGeneric* horner(F2LinRngGeneric* rng, GF2X* jump_poly) {
    F2LinRngGeneric* tmp = f2lin_rng_generic_init_zero();  
    size_t i = GF2X_deg(jump_poly);

    f2lin_rng_generic_copy(tmp, rng);

    if (i > 0) {
        f2lin_rng_generic_next_state(tmp);
        --i;
        for(; i > 0; --i) {
            if(GF2X_coeff(jump_poly, i) != 0) f2lin_rng_generic_add(tmp, rng);
            f2lin_rng_generic_next_state(tmp);
        }

        if(GF2X_coeff(jump_poly, 0) != 0) f2lin_rng_generic_add(tmp, rng);
    }

    f2lin_rng_generic_copy(rng, tmp);
    return rng;
}

static GF2X* load_min_poly() {
    GF2X* min_poly = GF2X_zero_init();
    char* min_poly_string = f2lin_rng_generic_min_poly();
    for (size_t i = 0; min_poly_string[i] != 0; ++i) {
        GF2X_SetCoeff(min_poly, i, min_poly_string[i] == '1' ? 1 : 0);
    }
    return min_poly;
}

char* test_verify_min_poly(void) {
    GF2X* min_poly = load_min_poly();
    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    // the minimal polynomial p(x) has to uphold the following condition:
    // p(A) == 0, meaning if we evaluate the p with the input as the transition
    // matrix, it has to be zero. Therefore p(A) * x == 0
    // We first generate a number and check that it is non zero,
    // than we evaluate the polynomial using horners rule, which should set the state to
    // all zeros, meaning if we generate a random number afterwards it should be zero

    mu_assert("Rng produces 0s, check if it is seeded correctly",
               f2lin_rng_generic_gen64(rng));

    horner(rng, min_poly);

    mu_assert("p(A) * x is not zero, the minimal polynomial is wrong",
               !f2lin_rng_generic_gen64(rng));


    return EXIT_SUCCESS;
}

static char* all_tests() {
    mu_run_test(test_verify_min_poly);

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
