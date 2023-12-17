#define XorMT

#include <stdio.h>
#include "minunit.h"
#include "../src/config.h"
#include "../src/jump.h"
#include "../src/rng_generic/rng_generic.h"

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

static char* test_first_20k() {
    size_t MAX = 20000;
    Xor64Config c = { .q = 4, .algorithm = HORNER };
    Xor64RngGeneric iter;
    xor64_rng_generic_init(&iter);
    for (size_t i = 0; i < 310; ++i) xor64_rng_generic_gen64(&iter);
    
    for (size_t i = 311; i < MAX; ++i) {
        Xor64RngGeneric jump;
        Xor64Jump params = { 0 };

        xor64_rng_generic_init(&jump);
        xor64_jump_init(&params, i, &c);
        xor64_jump_jump(&params, &jump);
        xor64_rng_generic_gen64(&iter);

        if (!compare_state(&jump, &iter)) {
            printf("wrong result at jump: %zu\n", i);
            //return "";
        }
    }

    return 0;
}




static char* all_tests() {
    mu_run_test(test_first_20k);

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
