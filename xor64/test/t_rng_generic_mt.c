#define XorMT

#include <stdio.h>
#include "minunit.h"
#include "../src/rng_generic/rng_generic.h"

typedef Xor64RngGeneric Rng;
int tests_run = 0;

static void fill_mt(Rng* mt, uint64_t shift) {
    uint64_t* state = &mt->mt.mt[0];
    for (size_t i = shift; i < NN + shift; ++i) {
        mt->mt.mt[(mt->mt.mti + i - shift) % NN] = (1ull << (i % 64));
    }
}

static int compare_states(Rng* a, Rng* b) {
    for (size_t i = 0; i < NN; ++i) {
        if (a->mt.mt[(a->mt.mti + i) % NN] != b->mt.mt[(b->mt.mti + i) % NN]) return 0;
    }

    return 1;
}

static char* test_add_zeros() {
    // test with zero mt
    Rng src = { 0 };
    Rng dest = { 0 };
    src.mt.mti = 0;
    dest.mt.mti = 123;

    fill_mt(&src, 0);

    xor64_rng_generic_add(&dest, &src);

    mu_assert("Adding to all 0s gives wrong result", 
            compare_states(&dest, &src));
    
    return 0;
}

static void print_state(Rng* a) {
    for (size_t i = 0; i < NN; ++i) {
        uint64_t n = a->mt.mt[i];
        for (int j = 63; j >= 0; --j) {
            printf("%llu", (n >> j) & 1);
        }
        printf(" ");
    }
    printf("\n\n");
}
static char* test_add_both_init() {
    Rng src = { 0 };
    Rng dest = { 0 };

    src.mt.mti = 0;
    dest.mt.mti = 0;

    fill_mt(&src, 1);
    fill_mt(&dest, 0);
    print_state(&src);
    print_state(&dest);

    xor64_rng_generic_add(&dest, &src);

    print_state(&dest);

    for (size_t i = 0; i < NN; ++i) {
        if (dest.mt.mt[(dest.mt.mti + i) % NN] != 
                ((1ull << (i % 64)) ^ src.mt.mt[(src.mt.mti + i) % NN])) {
            return "wrong result when addin both initialized\n";
        }
    }

    return 0;
}

static char* all_tests() {
    mu_run_test(test_add_zeros);
    mu_run_test(test_add_both_init);

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
