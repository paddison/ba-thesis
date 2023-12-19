#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "gray.h"
#include "jump_ahead.h"
#include "gf2x_wrapper.h"
#include "rng_generic/rng_generic.h"
#include "poly_decomp.h"

/*------------------------------------------------------ 
 * Forward Declarations                                |
 /----------------------------------------------------*/

// functions used for initialization
static GF2X* init_jump_poly(const GF2X* min_poly, const size_t jump_size);
static void init_decomp_poly();
static GF2X* load_min_poly();

// verification
static void verify_config(Xor64Config* cfg);

// functions used for implementing the jump algorithm
static void init_sliding_window(const int Q, Xor64RngGeneric* h[1 << Q], Xor64RngGeneric* rng);
static void precompute_A(const int Q, Xor64RngGeneric* A[Q + 1], const Xor64RngGeneric* rng);
static void compute_decomposition_polys(const int Q, Xor64RngGeneric* h[1 << Q], const Xor64RngGeneric* A[Q + 1]);
static Xor64RngGeneric* horner(Xor64RngGeneric* rng, const GF2X* jump_poly);
static Xor64RngGeneric* horner_sliding_window(const int Q, Xor64RngGeneric* rng, const GF2X* jump_poly, const Xor64RngGeneric* h[1 << Q]);
static Xor64RngGeneric* horner_sliding_window_decomp(const int Q, Xor64RngGeneric* rng, const Xor64PolyDecomp* jump_poly, const Xor64RngGeneric* h[1 << Q]);


/*------------------------------------------------------ 
 * Header Implementations                              |
 /----------------------------------------------------*/

Xor64Jump* xor64_jump_ahead_init(size_t jump_size, Xor64Config* cfg) {
    // load the minimal polynomial from the header
    const GF2X* min_poly = load_min_poly();
    Xor64Jump* jump_params = calloc(1, sizeof(Xor64Jump));
    Xor64Config def = { .q = Q_DEFAULT, .algorithm = ALGORITHM_DEFAULT };

    // verify config
    if (!cfg) cfg = &def;
    else verify_config(cfg);

    // initialize config values
    jump_params->q = cfg->q;
    jump_params->algorithm = cfg->algorithm;
    jump_params->jump_poly = init_jump_poly(min_poly, jump_size);
    jump_params->h = malloc(sizeof(Xor64RngGeneric*) * (1 << jump_params->q));

    // initialize jump and decomposition polynomials
    jump_params->decomp_poly = xor64_poly_decomp_init_from_gf2x(jump_params->jump_poly, jump_params->q);
    GF2X_zero_destroy(min_poly);
    //GF2X_print(jump_params->jump_poly);
    return jump_params;
}


Xor64RngGeneric* xor64_jump_ahead_jump(Xor64Jump* jump_params, Xor64RngGeneric* rng) {
    if (jump_params->algorithm == HORNER) return horner(rng, jump_params->jump_poly);

    init_sliding_window(jump_params->q, jump_params->h, rng);

    if (jump_params->algorithm == SLIDING_WINDOW) {
        return horner_sliding_window(jump_params->q, 
                                     rng, 
                                     jump_params->jump_poly, 
                                     jump_params->h); 
    } else { 
        return horner_sliding_window_decomp(jump_params->q, 
                                            rng, 
                                            jump_params->decomp_poly, 
                                            jump_params->h);
    }
}

void xor64_jump_ahead_destroy(Xor64Jump* jump_params) {
    GF2X_zero_destroy(jump_params->jump_poly);
    xor64_poly_decomp_destroy(jump_params->decomp_poly);
    for (size_t i = 0; i < 1 << jump_params->q; ++i) {
        xor64_rng_generic_destroy(jump_params->h[i]);
    }
    free(jump_params->h);
    free(jump_params->decomp_poly);
    free(jump_params);
}

/*------------------------------------------------------ 
 * Internal Implementations                            |
 /----------------------------------------------------*/

static GF2X* init_jump_poly(const GF2X* min_poly, const size_t jump_size) {
    GF2X* jump_poly = GF2X_zero_init();
    GF2X* x = GF2X_zero_init();
    GF2X_SetCoeff(x, 1, 1);
    GF2XModulus* minimal_poly_mod = GF2XModulus_zero_init();

    GF2XModulus_build(minimal_poly_mod, min_poly);
    GF2X_PowerMod(jump_poly, x, jump_size, minimal_poly_mod);
    
    //GF2X_print(jump_poly);
    return jump_poly;
}

static GF2X* load_min_poly() {
    GF2X* min_poly = GF2X_zero_init();
    char* min_poly_string = xor64_rng_generic_min_poly();
    for (size_t i = 0; min_poly_string[i] != 0; ++i) {
        GF2X_SetCoeff(min_poly, i, min_poly_string[i] == '1' ? 1 : 0);
    }
    return min_poly;
}

static void verify_config(Xor64Config* cfg) {
    if (cfg->q > Q_MAX || cfg->q == 0)  {
        fprintf(stderr, "Invalid value for Q: %d, defaulting to 4", cfg->q);
        cfg->q = Q_DEFAULT;
    }
}

static void init_sliding_window(const int Q, Xor64RngGeneric* h[1 << Q], Xor64RngGeneric* rng) {
    // in the paper, the seed is denoted as 'x'
    Xor64RngGeneric* A[Q + 1]; // = A^j * x

    // precompute A^j * x 
    precompute_A(Q, A, rng);

    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    compute_decomposition_polys(Q, h, A);

    for (size_t i = 0; i < Q + 1; ++i) {
        xor64_rng_generic_destroy(A[i]);
    }

}

// precomputes A^0x..A^qx which are the first q state transitions
// (equivalent to repeated calls to step()
static void precompute_A(const int Q, Xor64RngGeneric* A[Q + 1], const Xor64RngGeneric* rng) {
    Xor64RngGeneric* tmp = xor64_rng_generic_init_zero();// = calloc(1, sizeof(*rng)); 
    xor64_rng_generic_copy(tmp, rng);


    for (size_t i = 0; i < Q + 1; ++i) { 
        A[i] = xor64_rng_generic_init_zero();
        xor64_rng_generic_copy(A[i], tmp);
        xor64_rng_generic_next_state(tmp);
    }
    xor64_rng_generic_destroy(tmp);
}

static void compute_decomposition_polys(const int Q, Xor64RngGeneric* h[1 << Q], 
                                      const Xor64RngGeneric* A[Q + 1]) {
    // A[Q] is the polynomial component which contains z^q,
    // which is always included, and thus used to initialize
    // the decomposition polys
    h[0] = xor64_rng_generic_init_zero();
    xor64_rng_generic_copy(h[0], A[Q]);
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        // find out which bit flips
        uint64_t diff = GRAY[i] ^ GRAY[i - 1];
        size_t set_bit = 0;

        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }

        h[GRAY[i]] = xor64_rng_generic_init_zero();
        xor64_rng_generic_copy(h[GRAY[i]], h[GRAY[i - 1]]);
        xor64_rng_generic_add(h[GRAY[i]], A[set_bit - 1]);
    }
}

// calculate jump polynomial by evaluating with horners method
static Xor64RngGeneric* horner(Xor64RngGeneric* rng, const GF2X* jump_poly) {
    Xor64RngGeneric* tmp = xor64_rng_generic_init_zero();  
    size_t i = GF2X_deg(jump_poly);

    xor64_rng_generic_copy(tmp, rng);

    if (i > 0) {
        xor64_rng_generic_next_state(tmp);
        --i;
        for(; i > 0; --i) {
            if(GF2X_coeff(jump_poly, i) != 0) xor64_rng_generic_add(tmp, rng);
            xor64_rng_generic_next_state(tmp);
        }

        if(GF2X_coeff(jump_poly, 0) != 0) xor64_rng_generic_add(tmp, rng);
    }

    xor64_rng_generic_copy(rng, tmp);
    return rng;
}

static Xor64RngGeneric* horner_sliding_window(int Q, Xor64RngGeneric* rng, const GF2X* jump_poly, const Xor64RngGeneric* h[1 << Q]) {
    // use horners method with sliding window
    Xor64RngGeneric* tmp = xor64_rng_generic_init_zero();
    int i = GF2X_deg(jump_poly); 
    int dj = i, dj_next = 0, dm = 0;

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component
    assert(GF2X_coeff(jump_poly, i) == 1);

    if (i >= (Q + 1)) {
        //; h1(A) * x, first component in horner's method
        xor64_rng_generic_copy(tmp, h[determine_gray_enumeration(Q, i, jump_poly)]);

        i -= (Q + 1);

        for (; i >= Q; --i) {
            if (GF2X_coeff(jump_poly, i) == 1) {
                dj_next = i;
                // step forward dj - dj_next steps, which is the same as
                // multiplying with A^(dj - dj_next)
                for (size_t j = 0; j < dj - dj_next; ++j) xor64_rng_generic_next_state(tmp);

                // find out the gray_enumeration of the current decomposition
                // polynomials and calculate cur_state + h_i(A)x
                xor64_rng_generic_add(tmp, h[determine_gray_enumeration(Q, i, jump_poly)]);

                i -= Q;
                dj = dj_next;
            }
        }

        // calculate h_m+1 and do the last part of the decomposition 
        // which is multiplying the current state with A^dm
        // xoring with h_m+1(A)x and adding A^qx
        dm = dj - Q;

        for (size_t j = 0; j < dm; ++j) xor64_rng_generic_next_state(tmp);
    }

    xor64_rng_generic_add(tmp, h[determine_gray_enumeration(i + 1, i + 1, jump_poly)]);
    xor64_rng_generic_add(tmp, h[0]);

    xor64_rng_generic_copy(rng, tmp);
    xor64_rng_generic_destroy(tmp);
    return rng;
}

static Xor64RngGeneric* horner_sliding_window_decomp(const int Q, Xor64RngGeneric* rng, const Xor64PolyDecomp* decomp_poly, const Xor64RngGeneric** h) {
    Xor64RngGeneric* tmp = xor64_rng_generic_init_zero();

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component

    //; h1(A) * x, first component in horner's method
    if (decomp_poly->m) {
        size_t dj = decomp_poly->params[0].d, dj_next;
        xor64_rng_generic_copy(tmp, h[decomp_poly->params[0].h]);

        for (size_t i = 1; i < decomp_poly->m; ++i) {
            dj_next = decomp_poly->params[i].d;
            for (size_t j = 0; j < dj - dj_next; ++j) xor64_rng_generic_next_state(tmp);
            xor64_rng_generic_add(tmp, h[decomp_poly->params[i].h]);
            dj = dj_next;
        }

        for (size_t i = 0; i < decomp_poly->params[decomp_poly->m - 1].d; ++i) xor64_rng_generic_next_state(tmp);
    }

    xor64_rng_generic_add(tmp, h[decomp_poly->hm1]);
    xor64_rng_generic_add(tmp, h[0]);
    xor64_rng_generic_copy(rng, tmp);

    xor64_rng_generic_destroy(tmp);

    return rng;
}
