#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "NTL/GF2X.h"

#include "rng_generic/rng_generic.h"

#define F_NAME "minpoly.h"

/* Forward Declarations */
static void f2lin_init_min_poly(NTL::GF2X& p_min);

/* Internal Implementations */
static void f2lin_init_min_poly(NTL::GF2X& p_min) {

    const int state_size = f2lin_rng_generic_state_size();
    const size_t seq_len = 2 * state_size ;
    std::cout << state_size << std::endl;

    NTL::vec_GF2 seq(NTL::INIT_SIZE, seq_len);
    F2LinRngGeneric* rng = f2lin_rng_generic_init();

    for (size_t i = 0; i < seq_len; ++i) {
        seq[i] = f2lin_rng_generic_next_state(rng) & 0x01ul;
    }

    NTL::MinPolySeq(p_min, seq, state_size);
}

// store the coefficients of polynomial of degree n in a string of the form
// a_0*x^n, a_1*x^(n-1) ... a_n*
// Note that the string is NOT null terminated
static char* f2lin_minpoly_to_string(NTL::GF2X& p_min) {
    int deg = NTL::deg(p_min);
    char* buf = (char*) malloc(sizeof(char) * deg);

    for (int i = deg; i >= 0; --i) { 
        NTL::coeff(p_min, i) == 1 ? buf[i] = '1' : buf[i] = '0';
    }

    return buf;
}

int main(void) {
    char* p_min_string;
    FILE* file;
    NTL::GF2X p_min;
    size_t p_min_len;

    /* initialize minimal polynomial and write it to string */
    printf("%s\n", F_NAME);
    f2lin_init_min_poly(p_min);
    p_min_len = NTL::deg(p_min) + 1;
    //std::cout << p_min << std::endl;
    p_min_string = f2lin_minpoly_to_string(p_min);

    file = fopen((char*) F_NAME, "w");
    if (!file) {
        printf("unable to open file");
        return EXIT_FAILURE;
    }
    fwrite("#define MIN_POLY \"", sizeof(char), 18, file);

    if (fwrite(p_min_string, sizeof(char), p_min_len, file) != p_min_len) {
        fprintf(stderr, "Didn't write out full polynomial\n");
        return EXIT_FAILURE;
    }

    fwrite("\"", sizeof(char), 1, file);

    return EXIT_SUCCESS;
}
