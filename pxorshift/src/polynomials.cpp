#include "NTL/GF2X.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cassert>
#include "polynomials.hpp"

#define SEED 123234345ull
#define Q_MAX 10

static int Q = 4;

static NTL::GF2X jump_poly;
static NTL::GF2X minimal_poly;
static PolyGF2Decomp decomp_poly = { .params = 0, .m = 0, .cap = 1, .hm1 = 0 };

static RngGeneric h[1 << Q_MAX];
static uint16_t gray[1 << Q_MAX];

static JumpAlgorithm algo = SLIDING_WINDOW;

/* Forward Declarations */
static void init_min_poly(NTL::GF2X& p_min);
static void init_jump_poly(NTL::GF2X& p_min, NTL::GF2X& p_jump, size_t jump_size);
static void init_decomp_poly();
static void init_sliding_window(RngGeneric* rng);

static void set_jump_algorithm(enum JumpAlgorithm algorithm);
static void set_q(size_t q);

static RngGeneric* horner(RngGeneric* rng);
static RngGeneric* precompute_A(size_t q, RngGeneric A[q + 1], const RngGeneric* rng);
static RngGeneric* compute_decomposition_polys(size_t q, RngGeneric h[1 << q], RngGeneric A[q + 1]);
static inline size_t determine_gray_enumeration(const size_t q, size_t i, const NTL::GF2X& poly);
static RngGeneric* horner_sliding_window(RngGeneric* rng);
static RngGeneric* horner_sliding_window_decomp(RngGeneric* rng);

static PolyGF2Decomp* poly_gf2_decomp_init(PolyGF2Decomp* pd, size_t deg, size_t q);
static int poly_gf2_decomp_add_param(PolyGF2Decomp* pd, DecompParam dp);

/* Header Implementations */
void polynomials_init(Config* config) {
    if (config) {
        set_q(config->q);
        set_jump_algorithm(config->algorithm);
    }
    init_min_poly(minimal_poly);
}

void polynomials_init_jump(size_t jump) {
    init_jump_poly(minimal_poly, jump_poly, jump);
    init_decomp_poly();
}

RngGeneric* polynomials_jump(RngGeneric* rng) {
    if (algo == HORNER) return horner(rng);

    init_sliding_window(rng);

    if (algo == SLIDING_WINDOW) return horner_sliding_window(rng); 
    else return horner_sliding_window_decomp(rng);
}

void set_jump_algorithm(enum JumpAlgorithm algorithm) {
    if (algorithm != DEFAULT) algo = algorithm;
}

void set_q(size_t q) {
    if (q > Q_MAX) {
        fprintf(stderr, 
                "Trying to set Q value larger than max: Input is: (%zu), max is: %d\n",
                q, Q_MAX);
    } else {
        Q = q;
    }
}

/* Internal Implementations */
static void init_min_poly(NTL::GF2X& p_min) {
    std::cout << STATE_SIZE << std::endl;
    const int STATE_LEN = STATE_SIZE;
    const size_t SEQ_LEN = 2 * STATE_SIZE;

    NTL::vec_GF2 seq(NTL::INIT_SIZE, SEQ_LEN);
    RngGeneric rng = { 0 };
    rng_generic_init(&rng);

    for (size_t i = 0; i < SEQ_LEN; ++i) {
        seq[i] = rng_generic_step(&rng) & 0x01ul;
    }

    NTL::MinPolySeq(p_min, seq, STATE_LEN);
}


static void init_jump_poly(NTL::GF2X& p_min, NTL::GF2X& p_jump, size_t jump_size) {
    NTL::GF2X x(NTL::INIT_MONO, 1);
    NTL::GF2XModulus minimal_poly_mod;

    NTL::build(minimal_poly_mod, p_min);
    NTL::PowerMod(p_jump, x, jump_size, minimal_poly_mod);
}

static void init_decomp_poly() {
    int i = NTL::deg(jump_poly);

    poly_gf2_decomp_init(&decomp_poly, NTL::deg(jump_poly), Q);

    for (; i >= (int) Q; --i) {
        if (NTL::coeff(jump_poly, i) == 0) continue;
        
        DecompParam dp = { 0 };
        dp.h = determine_gray_enumeration(Q, i, jump_poly);
        dp.d = i - Q;
        i -= Q;
        if (!poly_gf2_decomp_add_param(&decomp_poly, dp)) {
            fprintf(stderr, "Unable to allocate space for new parameter");
        }
    }

    decomp_poly.hm1 = determine_gray_enumeration(i + 1, i + 1, jump_poly);
}


static inline size_t determine_gray_enumeration(const size_t q, size_t i, const NTL::GF2X& poly) {
    size_t gray_enumeration = 0;

    for (size_t j = 0; j < q; ++j) {
        gray_enumeration = (gray_enumeration << 1) ^ NTL::rep(NTL::coeff(poly, i - j - 1));
    }
    
    return gray_enumeration;
}

// calculate jump polynomial by evaluating with horners method
static RngGeneric* horner(RngGeneric* rng) {
    RngGeneric tmp;  
    size_t i = NTL::deg(jump_poly);

    rng_generic_copy(&tmp, rng);

    if (i > 0) {
        rng_generic_next_state(&tmp);
        --i;
        for(; i > 0; --i) {
            if(NTL::coeff(jump_poly, i) != 0) rng_generic_add(&tmp, rng);
            rng_generic_next_state(&tmp);
        }

        if(NTL::coeff(jump_poly, 0) != 0) rng_generic_add(&tmp, rng);
    }

    rng_generic_copy(rng, &tmp);
    return rng;
}

static void init_sliding_window(RngGeneric* rng) {
    // in the paper, the seed is denoted as 'x'
    RngGeneric A[Q + 1]; // = A^j * x

    // do gray code enumeration
    gray_code(Q, gray);

    // precompute A^j * x 
    precompute_A(Q, A, rng);

    // do all the rest
    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    compute_decomposition_polys(Q, h, A);
}


// precomputes A^0x..A^qx which are the first q state transitions
// (equivalent to repeated calls to step()
static RngGeneric* precompute_A(size_t q, RngGeneric A[q + 1], const RngGeneric* rng) {
    RngGeneric tmp = { 0 }; 
    rng_generic_copy(&tmp, rng);

    for (size_t i = 0; i < Q + 1; ++i) { 
        rng_generic_copy(&A[i], &tmp);
        rng_generic_next_state(&tmp);
    }
    
    return A;
}

static RngGeneric* compute_decomposition_polys(size_t q, RngGeneric h[1 << q], 
                                      RngGeneric A[q + 1]) {
    // A[Q] is the polynomial component which contains z^q,
    // which is always included, and thus used to initialize
    // the decomposition polys
    rng_generic_copy(&h[0], &A[Q]);
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        // find out which bit flips
        uint64_t diff = gray[i] ^ gray[i - 1];
        size_t set_bit = 0;

        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }

        rng_generic_copy(&h[gray[i]], &h[gray[i - 1]]);
        rng_generic_add(&h[gray[i]], &A[set_bit - 1]);
    }

    return h;
}

static RngGeneric* horner_sliding_window(RngGeneric* rng) {
    // use horners method with sliding window
    RngGeneric tmp = { 0 };
    int i = NTL::deg(jump_poly); 
    int dj = i, dj_next = 0, dm = 0;

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component
    assert(NTL::coeff(jump_poly, i) == 1);

    if (i >= (Q + 1)) {
        //; h1(A) * x, first component in horner's method
        rng_generic_copy(&tmp, &h[determine_gray_enumeration(Q, i, jump_poly)]);

        i -= (Q + 1);

        for (; i >= Q; --i) {
            if (NTL::coeff(jump_poly, i) == 1) {
                dj_next = i;
                // step forward dj - dj_next steps, which is the same as
                // multiplying with A^(dj - dj_next)
                for (size_t j = 0; j < dj - dj_next; ++j) rng_generic_next_state(&tmp);

                // find out the gray_enumeration of the current decomposition
                // polynomials and calculate cur_state + h_i(A)x
                rng_generic_add(&tmp, &h[determine_gray_enumeration(Q, i, jump_poly)]);

                i -= Q;
                dj = dj_next;
            }
        }

        // calculate h_m+1 and do the last part of the decomposition 
        // which is multiplying the current state with A^dm
        // xoring with h_m+1(A)x and adding A^qx
        dm = dj - Q;

        for (size_t j = 0; j < dm; ++j) rng_generic_next_state(&tmp);
    }

    rng_generic_add(&tmp, &h[determine_gray_enumeration(i + 1, i + 1, jump_poly)]);
    rng_generic_add(&tmp, &h[0]);

    rng_generic_copy(rng, &tmp);
    return rng;
}

static RngGeneric* horner_sliding_window_decomp(RngGeneric* rng) {
    RngGeneric tmp = { 0 };

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component

    //; h1(A) * x, first component in horner's method
    if (decomp_poly.m) {
        size_t dj = decomp_poly.params[0].d, dj_next;
        rng_generic_copy(&tmp, &h[decomp_poly.params[0].h]);

        for (size_t i = 1; i < decomp_poly.m; ++i) {
            dj_next = decomp_poly.params[i].d;
            for (size_t j = 0; j < dj - dj_next; ++j) rng_generic_next_state(&tmp);
            rng_generic_add(&tmp, &h[decomp_poly.params[i].h]);
            dj = dj_next;
        }

        for (size_t i = 0; i < decomp_poly.params[decomp_poly.m - 1].d; ++i) rng_generic_next_state(&tmp);
    }

    rng_generic_add(&tmp, &h[decomp_poly.hm1]);
    rng_generic_add(&tmp, &h[0]);
    rng_generic_copy(rng, &tmp);

    return rng;
}

static PolyGF2Decomp* poly_gf2_decomp_init(PolyGF2Decomp* pd, size_t deg, size_t q) {
    // assuming p is of length k and on average has k/2 non zero components
    // and also half of the components of a subpoly of size q are non zero,
    // the expected size will be k / (2 * (q / 2)) = k / q
    if (q == 0) return 0;
    if (!pd) pd = (PolyGF2Decomp*) malloc(sizeof(PolyGF2Decomp));

    // there are only mallocs if cap 

    // if we're initializing the first time
    if (pd->m == 0) {
        pd->cap = deg > q ? deg / q : 16;
        pd->params = pd->cap ? (DecompParam*) malloc(sizeof(DecompParam) * pd->cap) : 0;
    }
    pd->m = 0;
    pd->hm1 = 0;

    return pd; 
}

static int poly_gf2_decomp_add_param(PolyGF2Decomp* pd, DecompParam dp) {
    if (!pd) return 0;

    if (pd->m == pd->cap) { 
        pd->cap <<= 1;
        DecompParam* params = (DecompParam*) realloc(pd->params, pd->cap * sizeof(DecompParam));
        if (!params) return 0;
        pd->params = params;
    }

    pd->params[pd->m++] = dp;

    return 1;
}


static void poly_gf2_print(size_t q, NTL::GF2X& p, PolyGF2Decomp* pd) {

    printf("Poly:\n");
    std::cout << p << std::endl;
    printf("\nDecomp:\n");

    for (size_t i = 0; i < pd->m; ++i) {
        int j = q;
        DecompParam dp = pd->params[i];
        printf("d%zu: %zu -> ", i + 1, dp.d);
        for (; j >= 0; --j) {
            printf("%u", dp.h & (1 << j) ? 1 : 0); 
        }
        printf("\n");
    }

    int i = q;
    uint16_t hm1  = pd->hm1;

    printf("h_m1: ");
    for (; i > -1; --i) {
        printf("%u", hm1 & (1 << i) ? 1 : 0); 
    }
    printf("\n");
}

