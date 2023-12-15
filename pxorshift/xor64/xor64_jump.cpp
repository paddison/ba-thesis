#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cassert>

#include "NTL/GF2X.h"

#include "xor64_jump.hpp"
#include "gray.hpp"
#include "minpoly.h"
#include "xor64_poly_decomp.hpp"

#define Q_MAX 10

/*------------------------------------------------------ 
 * Forward Declarations                                |
 /----------------------------------------------------*/

static void init_jump_poly(NTL::GF2X& jump_poly, const NTL::GF2X& min_poly, const size_t jump_size);
static void init_decomp_poly();
static void init_sliding_window(size_t Q, Xor64RngGeneric h[Q], Xor64RngGeneric* rng);

static NTL::GF2X load_min_poly();

static Xor64RngGeneric* horner(Xor64RngGeneric* rng, NTL::GF2X& jump_poly);
static Xor64RngGeneric* precompute_A(size_t q, Xor64RngGeneric A[q + 1], const Xor64RngGeneric* rng);
static Xor64RngGeneric* compute_decomposition_polys(size_t q, Xor64RngGeneric h[1 << q], Xor64RngGeneric A[q + 1]);
static Xor64RngGeneric* horner_sliding_window(const size_t q, Xor64RngGeneric* rng, const NTL::GF2X& jump_poly, Xor64RngGeneric h[q]);
static Xor64RngGeneric* horner_sliding_window_decomp(Xor64RngGeneric* rng, const Xor64PolyDecomp* jump_poly, Xor64RngGeneric h[]);


/*------------------------------------------------------ 
 * Header Implementations                              |
 /----------------------------------------------------*/

Xor64Jump* xor64_jump_init(Xor64Jump* jump_params, Xor64RngGeneric* rng, size_t jump_size, Config* c) {
    // load the minimal polynomial from the header
    const NTL::GF2X min_poly = load_min_poly();

    // initialize config values
    jump_params->q = c->q;
    jump_params->algorithm = c->algorithm;

    // initialize jump and decomposition polynomials
    init_jump_poly(jump_params->jump_poly, min_poly, jump_size);
    xor64_poly_decomp_init_from_gf2x(&jump_params->decomp_poly, jump_params->jump_poly, jump_params->q);
    return jump_params;
}


Xor64RngGeneric* xor64_jump_jump(Xor64Jump* jump_params, Xor64RngGeneric* rng) {
    if (jump_params->algorithm == HORNER) return horner(rng, jump_params->jump_poly);

    init_sliding_window(jump_params->q, jump_params->h, rng);

    if (jump_params->algorithm == SLIDING_WINDOW) {
        return horner_sliding_window(jump_params->q, 
                                     rng, 
                                     jump_params->jump_poly, 
                                     jump_params->h); 
    } else { 
        return horner_sliding_window_decomp(rng, 
                                            &jump_params->decomp_poly, 
                                            jump_params->h);
    }
}

void xor64_jump_destroy(Xor64Jump* jump_params) {
    xor64_poly_decomp_destroy(&jump_params->decomp_poly);
}

/*------------------------------------------------------ 
 * Internal Implementations                            |
 /----------------------------------------------------*/

static NTL::GF2X load_min_poly() {
    NTL::GF2X min_poly;
    for (size_t i = 0; MIN_POLY[i] != 0; ++i) {
        NTL::SetCoeff(min_poly, i, MIN_POLY[i] == '1' ? 1 : 0);
    }
    return min_poly;
}

static void init_jump_poly(NTL::GF2X& jump_poly, const NTL::GF2X& min_poly, const size_t jump_size) {
    NTL::GF2X x(NTL::INIT_MONO, 1);
    NTL::GF2XModulus minimal_poly_mod;

    NTL::build(minimal_poly_mod, min_poly);
    NTL::PowerMod(jump_poly, x, jump_size, minimal_poly_mod);
}

// calculate jump polynomial by evaluating with horners method
static Xor64RngGeneric* horner(Xor64RngGeneric* rng, NTL::GF2X& jump_poly) {
    Xor64RngGeneric tmp;  
    size_t i = NTL::deg(jump_poly);

    xor64_rng_generic_copy(&tmp, rng);

    if (i > 0) {
        xor64_rng_generic_next_state(&tmp);
        --i;
        for(; i > 0; --i) {
            if(NTL::coeff(jump_poly, i) != 0) xor64_rng_generic_add(&tmp, rng);
            xor64_rng_generic_next_state(&tmp);
        }

        if(NTL::coeff(jump_poly, 0) != 0) xor64_rng_generic_add(&tmp, rng);
    }

    xor64_rng_generic_copy(rng, &tmp);
    return rng;
}

static void init_sliding_window(size_t Q, Xor64RngGeneric h[1 << Q], Xor64RngGeneric* rng) {
    // in the paper, the seed is denoted as 'x'
    Xor64RngGeneric A[Q + 1]; // = A^j * x

    // precompute A^j * x 
    precompute_A(Q, A, rng);

    // do all the rest
    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    compute_decomposition_polys(Q, h, A);
}


// precomputes A^0x..A^qx which are the first q state transitions
// (equivalent to repeated calls to step()
static Xor64RngGeneric* precompute_A(size_t Q, Xor64RngGeneric A[Q + 1], const Xor64RngGeneric* rng) {
    Xor64RngGeneric tmp = { 0 }; 
    xor64_rng_generic_copy(&tmp, rng);

    for (size_t i = 0; i < Q + 1; ++i) { 
        xor64_rng_generic_copy(&A[i], &tmp);
        xor64_rng_generic_next_state(&tmp);
    }
    
    return A;
}

static Xor64RngGeneric* compute_decomposition_polys(size_t Q, Xor64RngGeneric h[1 << Q], 
                                      Xor64RngGeneric A[Q + 1]) {
    // A[Q] is the polynomial component which contains z^q,
    // which is always included, and thus used to initialize
    // the decomposition polys
    xor64_rng_generic_copy(&h[0], &A[Q]);
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        // find out which bit flips
        uint64_t diff = GRAY[i] ^ GRAY[i - 1];
        size_t set_bit = 0;

        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }

        xor64_rng_generic_copy(&h[GRAY[i]], &h[GRAY[i - 1]]);
        xor64_rng_generic_add(&h[GRAY[i]], &A[set_bit - 1]);
    }

    return h;
}

static Xor64RngGeneric* horner_sliding_window(size_t Q, Xor64RngGeneric* rng, const NTL::GF2X& jump_poly, Xor64RngGeneric h[Q]) {
    // use horners method with sliding window
    Xor64RngGeneric tmp = { 0 };
    int i = NTL::deg(jump_poly); 
    int dj = i, dj_next = 0, dm = 0;

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component
    assert(NTL::coeff(jump_poly, i) == 1);

    if (i >= (Q + 1)) {
        //; h1(A) * x, first component in horner's method
        xor64_rng_generic_copy(&tmp, &h[determine_gray_enumeration(Q, i, jump_poly)]);

        i -= (Q + 1);

        for (; i >= Q; --i) {
            if (NTL::coeff(jump_poly, i) == 1) {
                dj_next = i;
                // step forward dj - dj_next steps, which is the same as
                // multiplying with A^(dj - dj_next)
                for (size_t j = 0; j < dj - dj_next; ++j) xor64_rng_generic_next_state(&tmp);

                // find out the gray_enumeration of the current decomposition
                // polynomials and calculate cur_state + h_i(A)x
                xor64_rng_generic_add(&tmp, &h[determine_gray_enumeration(Q, i, jump_poly)]);

                i -= Q;
                dj = dj_next;
            }
        }

        // calculate h_m+1 and do the last part of the decomposition 
        // which is multiplying the current state with A^dm
        // xoring with h_m+1(A)x and adding A^qx
        dm = dj - Q;

        for (size_t j = 0; j < dm; ++j) xor64_rng_generic_next_state(&tmp);
    }

    xor64_rng_generic_add(&tmp, &h[determine_gray_enumeration(i + 1, i + 1, jump_poly)]);
    xor64_rng_generic_add(&tmp, &h[0]);

    xor64_rng_generic_copy(rng, &tmp);
    return rng;
}

static Xor64RngGeneric* horner_sliding_window_decomp(Xor64RngGeneric* rng, const Xor64PolyDecomp* decomp_poly, Xor64RngGeneric h[]) {
    Xor64RngGeneric tmp = { 0 };

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component

    //; h1(A) * x, first component in horner's method
    if (decomp_poly->m) {
        size_t dj = decomp_poly->params[0].d, dj_next;
        xor64_rng_generic_copy(&tmp, &h[decomp_poly->params[0].h]);

        for (size_t i = 1; i < decomp_poly->m; ++i) {
            dj_next = decomp_poly->params[i].d;
            for (size_t j = 0; j < dj - dj_next; ++j) xor64_rng_generic_next_state(&tmp);
            xor64_rng_generic_add(&tmp, &h[decomp_poly->params[i].h]);
            dj = dj_next;
        }

        for (size_t i = 0; i < decomp_poly->params[decomp_poly->m - 1].d; ++i) xor64_rng_generic_next_state(&tmp);
    }

    xor64_rng_generic_add(&tmp, &h[decomp_poly->hm1]);
    xor64_rng_generic_add(&tmp, &h[0]);
    xor64_rng_generic_copy(rng, &tmp);

    return rng;
}
