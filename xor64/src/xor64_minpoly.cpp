#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "NTL/GF2X.h"

#if defined(Xor64)
    #include "xor64_rng_generic_64.h"
#elif defined(XorMT)
#else 
    #include "xor64_rng_generic_64.h"
#endif

#define F_NAME "minpoly.h"

/* Forward Declarations */
static void xor64_init_min_poly(NTL::GF2X& p_min);

/* Internal Implementations */
static void xor64_init_min_poly(NTL::GF2X& p_min) {
    const int state_size = XOR64_RNG_STATE_SIZE;
    const size_t seq_len = 2 * state_size ;

    NTL::vec_GF2 seq(NTL::INIT_SIZE, seq_len);
    Xor64RngGeneric rng = { 0 };
    xor64_rng_generic_init(&rng);

    for (size_t i = 0; i < seq_len; ++i) {
        seq[i] = xor64_rng_generic_gen64(&rng) & 0x01ul;
    }

    NTL::MinPolySeq(p_min, seq, state_size);
}

// store the coefficients of polynomial of degree n in a string of the form
// a_0*x^n, a_1*x^(n-1) ... a_n*
// Note that the string is NOT null terminated
static char* xor64_minpoly_to_string(NTL::GF2X& p_min) {
    int deg = NTL::deg(p_min);
    char* buf = (char*) malloc(sizeof(char) * deg);

    for (int i = deg; i >= 0; --i) { 
        NTL::coeff(p_min, i) == 1 ? buf[i] = '1' : buf[i] = '0';
    }

    return buf;
}

int main(void) {
    char* file_name = (char*) "minpoly.h"; 
    char* p_min_string;
    FILE* file;
    NTL::GF2X p_min;
    size_t p_min_len;

    /* initialize minimal polynomial and write it to string */
    xor64_init_min_poly(p_min);
    p_min_len = NTL::deg(p_min) + 1;
    p_min_string = xor64_minpoly_to_string(p_min);

    file = fopen((char*) file_name, "w");
    fwrite("#define MIN_POLY \"", sizeof(char), 18, file);

    if (fwrite(p_min_string, sizeof(char), p_min_len, file) != p_min_len) {
        fprintf(stderr, "Didn't write out full polynomial\n");
        return EXIT_FAILURE;
    }

    fwrite("\"", sizeof(char), 1, file);

    return EXIT_SUCCESS;
}
