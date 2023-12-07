#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "c_poly.h"

int tests_run = 0;

// highest degree coeff is on right
static PolyGF2 c_poly_from_str(char* poly_repr) {
    PolyGF2 p;
    
    p.deg = strlen(poly_repr) - 1;
    p.p = calloc(sizeof(uint64_t), p.deg / 64 + 1);

    for (int i = p.deg; i > -1; --i) {
        if (poly_repr[i] == '1') p.p[i / 64] |= (1ull << (i % 64));
    }

    return p;
}

static char* test_c_poly_from_str_deg_4() {
    PolyGF2 p = c_poly_from_str("1101");
    mu_assert("Wrong coefficients in p", p.p[0] == 11);

    return 0;
}

static char* test_c_poly_from_str_deg_65() {
    PolyGF2 p = c_poly_from_str("101101111111011100000111000011100110111111101110000011100001101011");
    mu_assert("Wrong coefficients in p", 
            p.p[1] == 3 && p.p[0] == 6372725373068701677);

    return 0;
}

static char* test_c_poly_from_str_deg_is_correct() {
    PolyGF2 p = c_poly_from_str("1000111000101111");
    mu_assert("P has wrong degree", p.deg == 15);

    return 0;
}

static char* test_c_poly_coeff() {
    PolyGF2 p = c_poly_from_str("1000111000101111");

    mu_assert("wrong coeff at x^15: expected 1", poly_gf2_coeff(&p, 15) == 1);
    mu_assert("wrong coeff at x^14: expected 1", poly_gf2_coeff(&p, 14) == 1);
    mu_assert("wrong coeff at x^13: expected 1", poly_gf2_coeff(&p, 13) == 1);
    mu_assert("wrong coeff at x^12: expected 1", poly_gf2_coeff(&p, 12) == 1);
    mu_assert("wrong coeff at x^11: expected 0", poly_gf2_coeff(&p, 11) == 0);
    mu_assert("wrong coeff at x^10: expected 1", poly_gf2_coeff(&p, 10) == 1);
    mu_assert("wrong coeff at x^9: expected 0", poly_gf2_coeff(&p, 9) == 0);
    mu_assert("wrong coeff at x^8: expected 0", poly_gf2_coeff(&p, 8) == 0);
    mu_assert("wrong coeff at x^7: expected 0", poly_gf2_coeff(&p, 7) == 0);
    mu_assert("wrong coeff at x^6: expected 1", poly_gf2_coeff(&p, 6) == 1);
    mu_assert("wrong coeff at x^5: expected 1", poly_gf2_coeff(&p, 5) == 1);
    mu_assert("wrong coeff at x^4: expected 1", poly_gf2_coeff(&p, 4) == 1);
    mu_assert("wrong coeff at x^3: expected 0", poly_gf2_coeff(&p, 3) == 0);
    mu_assert("wrong coeff at x^2: expected 0", poly_gf2_coeff(&p, 2) == 0);
    mu_assert("wrong coeff at x^1: expected 0", poly_gf2_coeff(&p, 1) == 0);
    mu_assert("wrong coeff at x^0: expected 1", poly_gf2_coeff(&p, 0) == 1);

    return 0;
}

static char* test_c_poly_coeff_larger_than_64() {
    PolyGF2 p = c_poly_from_str("101101111111011100000111000011100110111111101110000011100001101011");

    mu_assert("wrong coeff at x^65: expected 1", poly_gf2_coeff(&p, 65) == 1);
    mu_assert("wrong coeff at x^64: expected 1", poly_gf2_coeff(&p, 64) == 1);
    mu_assert("wrong coeff at x^63: expected 0", poly_gf2_coeff(&p, 63) == 0);

    return 0;
}

static char* test_c_poly_gf2_decomp() {
    PolyGF2 p = c_poly_from_str("1000111000101111");

    PolyGF2Decomp decomp = poly_gf2_decomp(&p, 3);

    mu_assert("wrong amount of subpolys", decomp.m == 3);
    mu_assert("h_1 is wrong, expected 0b111", decomp.params[0].h == 7);
    mu_assert("d1 is wrong, expected 12", decomp.params[0].d == 12);

    mu_assert("h_2 is wrong, expected 0b0", decomp.params[1].h == 0);
    mu_assert("d2 is wrong, expected 12", decomp.params[1].d == 7);

    mu_assert("h_3 is wrong, expected 0b110", decomp.params[2].h == 6);
    mu_assert("d3 is wrong, expected 12", decomp.params[2].d == 3);

    mu_assert("h_m1 is wrong, expected 0b1", decomp.hm1 == 1);

    return 0;
}

static char* all_tests() {
    mu_run_test(test_c_poly_from_str_deg_4);
    mu_run_test(test_c_poly_from_str_deg_65);
    mu_run_test(test_c_poly_from_str_deg_is_correct);
    mu_run_test(test_c_poly_coeff);
    mu_run_test(test_c_poly_coeff_larger_than_64);
    mu_run_test(test_c_poly_gf2_decomp);

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
