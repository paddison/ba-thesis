/* 
 * Unit tests for prand48. 
 *
 * In principle, it should behave exactly as drand48.
 * Thus some tests will test for identical results between drant48 and prand48.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "minunit.h"
#include "prand48.h"
#include "prand48.c"

int tests_run = 0;

static void IEEE754Double_dbg(union IEEE754Double * n) {
    printf("seeeeeeeeeeemmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm\n");
    for (size_t i = 0; i < 8; ++i) {
        for(size_t j = 0; j < 8; ++j) {
            printf("%d", (n->b[i] >> (7 - j)) & 1);
        }
    }
    printf("\n");
}

static char* test_IEEE754Double_range_0_to_1() {
    union IEEE754Double n = { 0 };

    IEEE754Double_new(&n,
                      0, 
                      0x3fe, 
                      (uint16_t[3]) { 0xffff, 0xffff, 0xffff });

    mu_assert("Numbers not equal", 9.9999999999999822364316059975E-1 == n.d);

    return 0;
}

static char* test_IEEE754Double_neg_2_375() {
    union IEEE754Double n = { 0 };

    IEEE754Double_new(&n, 
                      1, 
                      0x400, 
                      (uint16_t[3]) { 0, 0, 0x3000 });

    mu_assert("Number not -2.375", -2.375 == n.d);

    return 0;
}

static char* test_seek_equals_iterate() {
    prand48_init();
    
    uint16_t iter[3] = { 0 };
    uint16_t seek[3] = { 0 };
    prand48_jump(iter, 0);
    prand48_jump(seek, 0);
    // compare the first ten million numbers 
    for (uint64_t i = 1; i < 10000000; ++i) {
        __prand_next(iter);
        prand48_jump(seek, i);

        mu_assert("iterate and seek generate different numbers", 
                   iter[0] == seek[0] && 
                   iter[1] == seek[1] && 
                   iter[2] == seek[2]);
    }

    return 0;
}

static char* test_pdrand_equals_drand() {
    prand48_init();
    uint16_t n[3] = { 0 };
    seed48((uint16_t[3]) { 0x1234, 0xabcd, 0x330e });

    prand48_jump(n, 0);

    for (uint64_t i = 1; i < 100; ++i) {
        double prand = pdrand48(n);
        double drand = drand48();
        mu_assert("Parallel version produces different numbers than mrand48",
                   prand == drand);
    }

    return 0;
}

static char* test_plrand_equals_lrand() {
    prand48_init();
    uint16_t n[3] = { 0 };
    seed48((uint16_t[3]) { 0x1234, 0xabcd, 0x330e });

    prand48_jump(n, 0);

    for (uint64_t i = 1; i < 100; ++i) {
        uint32_t prand = plrand48(n);
        uint32_t drand = lrand48();
        mu_assert("Parallel version produces different numbers than mrand48",
                   prand == drand);
    }

    return 0;
}

static char* test_pmrand_equals_mrand() {
    prand48_init();
    uint16_t n[3] = { 0 };
    seed48((uint16_t[3]) { 0x1234, 0xabcd, 0x330e });

    prand48_jump(n, 0);

    for (uint64_t i = 1; i < 100; ++i) {
        uint32_t prand = pmrand48(n);
        uint32_t drand = mrand48();
        mu_assert("Parallel version produces different numbers than mrand48",
                   prand == drand);
    }

    return 0;
}

static char* test_algorithm_c() {
    uint64_t c = 1, g = 2, k = 10;
    uint64_t result = __algorithm_c(c, g, k);
    mu_assert("algorithm for evaluating sum is incorrect", 
               result == 1023);

    return 0;
}

static char* test_seek_intern_equals_seek_intern2() {
    uint64_t r = 20017429951246, a = 25214903917, c = 11, n = 250000;
    mu_assert("seek functions give different results",
              __seek_intern(a, c, n, r) == __seek_intern2(a, c, n, r));

    return 0;
}

static char* all_tests() {
    mu_run_test(test_IEEE754Double_range_0_to_1);
    mu_run_test(test_IEEE754Double_neg_2_375);
    mu_run_test(test_seek_equals_iterate);
    mu_run_test(test_pdrand_equals_drand);
    mu_run_test(test_plrand_equals_lrand);
    mu_run_test(test_pmrand_equals_mrand);
    mu_run_test(test_algorithm_c);
    mu_run_test(test_seek_intern_equals_seek_intern2);

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
