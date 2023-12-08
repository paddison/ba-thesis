#include "NTL/GF2X.h"
#include "minunit.h"
#include "prepare_jump.cpp"
#include "c_poly.h"

int tests_run = 0;

// highest degree coeff is on right

static char* test_ntl_to_c_polygf2() {
    NTL::GF2X p_ntl;
    //poly, i, val
    NTL::SetCoeff(p_ntl, 0, 1);
    NTL::SetCoeff(p_ntl, 4, 1);
    NTL::SetCoeff(p_ntl, 5, 1);
    NTL::SetCoeff(p_ntl, 6, 1);
    NTL::SetCoeff(p_ntl, 10, 1);
    NTL::SetCoeff(p_ntl, 12, 1);
    NTL::SetCoeff(p_ntl, 13, 1);
    NTL::SetCoeff(p_ntl, 14, 1);
    NTL::SetCoeff(p_ntl, 15, 1);

    PolyGF2 p_c = _gf2x_to_c_poly_gf2(p_ntl);

    mu_assert("NTL and C poly have different degree", 
            NTL::deg(p_ntl) == poly_gf2_deg(&p_c));

    for (size_t i = 0; i <= NTL::deg(p_ntl); ++i) {
        mu_assert("NTL and C poly have different coefficients",
                NTL::coeff(p_ntl, i) == poly_gf2_coeff(&p_c, i));
    }

    return 0;
}

static char* test_ntl_to_c_poly_large_deg() {
    NTL::GF2X p_ntl;

    NTL::SetCoeff(p_ntl, 1, 1);
    NTL::SetCoeff(p_ntl, 65, 1);
    NTL::SetCoeff(p_ntl, 122, 1);
    NTL::SetCoeff(p_ntl, 128, 1);
    NTL::SetCoeff(p_ntl, 1023, 1);
    
    PolyGF2 p_c = _gf2x_to_c_poly_gf2(p_ntl);

    for (size_t i = 0; i <= NTL::deg(p_ntl); ++i) {
        mu_assert("NTL and C poly have different coefficients for large degree",
                NTL::coeff(p_ntl, i) == poly_gf2_coeff(&p_c, i));
    }

}

static char* all_tests() {
    mu_run_test(test_ntl_to_c_polygf2);
    mu_run_test(test_ntl_to_c_poly_large_deg);

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
