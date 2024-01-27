#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "config.h"
#include "gray.h"
#include "jump_ahead.h"
#include "gf2x_wrapper.h"
#include "rng_generic/rng_generic.h"
#include "poly_decomp.h"

/*------------------------------------------------------ 
 * Forward Declarations                                |
 /----------------------------------------------------*/

// functions used for initialization
static 
GF2X* init_jump_poly(const GF2X* min_poly, const size_t jump_size);

static 
F2LinRngGeneric** init_y(int q); 

static 
void init_decomp_poly();

static 
void destroy_y(F2LinRngGeneric** y, int q);

static 
GF2X* load_min_poly();

// verification
static 
void verify_config(F2LinConfig* cfg);

// functions used for implementing the jump algorithm
static 
void init_sliding_window(const int Q, F2LinRngGeneric* h[1 << Q], 
                         F2LinRngGeneric* rng);

static 
void precompute_A(const int Q, F2LinRngGeneric* A[Q + 1], 
                  const F2LinRngGeneric* rng);

static 
void compute_decomposition_polys(const int Q, F2LinRngGeneric* h[1 << Q], 
                                 const F2LinRngGeneric* A[Q + 1]);

static 
F2LinRngGeneric* horner(F2LinRngGeneric* rng, const GF2X* jump_poly);

static 
F2LinRngGeneric* sliding_window(const int Q, F2LinRngGeneric* rng, 
                                const GF2X* jump_poly, const F2LinRngGeneric* h[1 << Q]);

static 
F2LinRngGeneric* sliding_window_decomp(const int Q, F2LinRngGeneric* rng, 
                                       const F2LinPolyDecomp* jump_poly, 
                                       const F2LinRngGeneric* h[1 << Q]);


/*------------------------------------------------------ 
 * Header Implementations                              |
 /----------------------------------------------------*/

F2LinJump* f2lin_jump_ahead_init(size_t jump_size, F2LinConfig* cfg) {
    // load the minimal polynomial from the header
    GF2X* min_poly = load_min_poly();
    F2LinJump* jump_params = calloc(1, sizeof(F2LinJump));
    F2LinConfig def = { .q = Q_DEFAULT, .algorithm = SLIDING_WINDOW_DECOMP };
    GF2X* jump_poly = init_jump_poly(min_poly, jump_size);
    union F2LinJumpPoly jp;

    // verify config
    if (!cfg) cfg = &def;
    else verify_config(cfg);

    // initialize config values
    jump_params->algorithm = cfg->algorithm;

    switch (jump_params->algorithm) {
        case HORNER:
            jp.horner = jump_poly; 
            break;
        case SLIDING_WINDOW:
            jp.sw = (F2LinJumpSW) {
                .q = cfg->q,
                .y = init_y(cfg->q),
                .jp = jump_poly,
            };
            break;
        default:
            jp.swd = (F2LinJumpSWD) {
                .q = cfg->q,
                .y = init_y(cfg->q),
                .pd = f2lin_poly_decomp_init_from_gf2x(jump_poly, cfg->q),
            };
            GF2X_zero_destroy(jump_poly);
    }

    jump_params->jp = jp;
    GF2X_zero_destroy(min_poly);
    return jump_params;
}

F2LinRngGeneric* f2lin_jump_ahead_jump(F2LinJump* jump_params, F2LinRngGeneric* rng) {
    switch (jump_params->algorithm) {
        case HORNER: 
            return horner(rng, jump_params->jp.horner);
        case SLIDING_WINDOW: {
            F2LinJumpSW* sw = &jump_params->jp.sw;
            init_sliding_window(sw->q, sw->y, rng);
            return sliding_window(sw->q, rng, sw->jp, sw->y);
        }
        default: {
            F2LinJumpSWD* swd = &jump_params->jp.swd;
            init_sliding_window(swd->q, swd->y, rng);
            return sliding_window_decomp(swd->q, rng, swd->pd, swd->y);
        }
    }
}

void f2lin_jump_ahead_destroy(F2LinJump* jump_params) {
    switch (jump_params->algorithm) {
        case HORNER: 
            GF2X_zero_destroy(jump_params->jp.horner); 
            break;
        case SLIDING_WINDOW: {
            F2LinJumpSW* sw = &jump_params->jp.sw;
            GF2X_zero_destroy(sw->jp);
            destroy_y(sw->y, sw->q);
            break;
        }
        default: {
            F2LinJumpSWD* swd = &jump_params->jp.swd;
            f2lin_poly_decomp_destroy(swd->pd);
            destroy_y(swd->y, swd->q);
        }
    }

    free(jump_params);
    jump_params = 0;
}

/*------------------------------------------------------ 
 * Internal Implementations                            |
 /----------------------------------------------------*/

static 
GF2X* init_jump_poly(const GF2X* min_poly, const size_t jump_size) {
    GF2X* jump_poly = GF2X_zero_init();
    GF2X* x = GF2X_zero_init();
    GF2X_SetCoeff(x, 1, 1);
    GF2XModulus* minimal_poly_mod = GF2XModulus_zero_init();

    GF2XModulus_build(minimal_poly_mod, min_poly);
    GF2X_PowerMod(jump_poly, x, jump_size, minimal_poly_mod);
    
    GF2X_zero_destroy(x);
    GF2XModulus_destroy(minimal_poly_mod);
    return jump_poly;
}

static 
F2LinRngGeneric** init_y(int q) {
    F2LinRngGeneric** y = calloc(sizeof(F2LinRngGeneric*), (1 << q));
    for (size_t i = 0; i < (1 << q); ++i) {
        y[i] = f2lin_rng_generic_init();
    }
    return y;
}

static 
void destroy_y(F2LinRngGeneric** y, int q) {
    for (size_t i = 0; i < (1 << q); ++i) {
        f2lin_rng_generic_destroy(y[i]);
    }
    free(y);
}

static 
GF2X* load_min_poly() {
    GF2X* min_poly = GF2X_zero_init();
    char* min_poly_string = f2lin_rng_generic_min_poly();
    for (size_t i = 0; min_poly_string[i] != 0; ++i) {
        GF2X_SetCoeff(min_poly, i, min_poly_string[i] == '1' ? 1 : 0);
    }
    return min_poly;
}

static 
void verify_config(F2LinConfig* cfg) {
    if (cfg->q > Q_MAX || cfg->q == 0)  {
        fprintf(stderr, "Invalid value for Q: %d, defaulting to 4", cfg->q);
        cfg->q = Q_DEFAULT;
    }
}

static 
void init_sliding_window(const int Q, F2LinRngGeneric* h[1 << Q], F2LinRngGeneric* rng) {
    // in the paper, the seed is denoted as 'x'
    F2LinRngGeneric* A[Q + 1]; // = A^j * x

    // precompute A^j * x 
    precompute_A(Q, A, rng);

    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    compute_decomposition_polys(Q, h, A);

    for (size_t i = 0; i < Q + 1; ++i) {
        f2lin_rng_generic_destroy(A[i]);
    }
}

// precomputes A^0x..A^qx which are the first q state transitions
// (equivalent to repeated calls to step()
static 
void precompute_A(const int Q, F2LinRngGeneric* A[Q + 1], const F2LinRngGeneric* rng) {
    F2LinRngGeneric* tmp = f2lin_rng_generic_init_zero();// = calloc(1, sizeof(*rng)); 
    f2lin_rng_generic_copy(tmp, rng);

    for (size_t i = 0; i < Q + 1; ++i) { 
        A[i] = f2lin_rng_generic_init_zero();
        f2lin_rng_generic_copy(A[i], tmp);
        f2lin_rng_generic_next_state(tmp);
    }
    f2lin_rng_generic_destroy(tmp);
}

static 
void compute_decomposition_polys(const int Q, F2LinRngGeneric* h[1 << Q], 
                                 const F2LinRngGeneric* A[Q + 1]) {
    // A[Q] is the polynomial component which contains z^q,
    // which is always included, and thus used to initialize
    // the decomposition polys
    f2lin_rng_generic_copy(h[0], A[Q]);
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        // find out which bit flips
        uint64_t diff = GRAY[i] ^ GRAY[i - 1];
        size_t set_bit = 0;

        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }

        f2lin_rng_generic_copy(h[GRAY[i]], h[GRAY[i - 1]]);
        f2lin_rng_generic_add(h[GRAY[i]], A[set_bit - 1]);
    }
}

// calculate jump polynomial by evaluating with horners method
static 
F2LinRngGeneric* horner(F2LinRngGeneric* rng, const GF2X* jump_poly) {
    F2LinRngGeneric* tmp = f2lin_rng_generic_init_zero();  
    int i = GF2X_deg(jump_poly);

    f2lin_rng_generic_copy(tmp, rng);

    if (i > 0) {
        f2lin_rng_generic_next_state(tmp);
        --i;
        for(; i > 0; --i) {
            if(GF2X_coeff(jump_poly, i) != 0) f2lin_rng_generic_add(tmp, rng);
            f2lin_rng_generic_next_state(tmp);
        }

        if(GF2X_coeff(jump_poly, 0) != 0) f2lin_rng_generic_add(tmp, rng);
    }

    f2lin_rng_generic_copy(rng, tmp);
    f2lin_rng_generic_destroy(tmp);
    return rng;
}

static 
F2LinRngGeneric* sliding_window(int Q, F2LinRngGeneric* rng, const GF2X* jump_poly, 
                                const F2LinRngGeneric* h[1 << Q]) {
    // use horners method with sliding window
    F2LinRngGeneric* tmp = f2lin_rng_generic_init_zero();
    int i = GF2X_deg(jump_poly); 
    int dj = i, dj_next = 0, dm = 0;

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component
    //assert(GF2X_coeff(jump_poly, i) == 1);

    if (i >= (Q)) {
        //; h1(A) * x, first component in horner's method
        f2lin_rng_generic_copy(tmp, h[f2lin_determine_gray_enumeration(Q, i, jump_poly)]);

        i -= (Q + 1);

        for (; i >= Q; --i) {
            if (GF2X_coeff(jump_poly, i) == 1) {
                dj_next = i;
                // step forward dj - dj_next steps, which is the same as
                // multiplying with A^(dj - dj_next)
                for (size_t j = 0; j < dj - dj_next; ++j) f2lin_rng_generic_next_state(tmp);

                // find out the gray_enumeration of the current decomposition
                // polynomials and calculate cur_state + h_i(A)x
                f2lin_rng_generic_add(tmp, h[f2lin_determine_gray_enumeration(Q, i, jump_poly)]);

                i -= Q;
                dj = dj_next;
            }
        }

        // calculate h_m+1 and do the last part of the decomposition 
        // which is multiplying the current state with A^dm
        // xoring with h_m+1(A)x and adding A^qx
        dm = dj - Q;

        for (size_t j = 0; j < dm; ++j) f2lin_rng_generic_next_state(tmp);
    }

    f2lin_rng_generic_add(tmp, h[f2lin_determine_gray_enumeration(i + 1, i + 1, jump_poly)]);
    f2lin_rng_generic_add(tmp, h[0]);

    f2lin_rng_generic_copy(rng, tmp);
    f2lin_rng_generic_destroy(tmp);
    return rng;
}

static 
F2LinRngGeneric* sliding_window_decomp(const int Q, F2LinRngGeneric* rng, 
                                       const F2LinPolyDecomp* decomp_poly, 
                                       const F2LinRngGeneric** h) {
    F2LinRngGeneric* tmp = f2lin_rng_generic_init_zero();

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component

    //; h1(A) * x, first component in horner's method
    if (decomp_poly->m) {
        f2lin_rng_generic_copy(tmp, h[decomp_poly->h[0]]);

        for (size_t i = 1; i < decomp_poly->m; ++i) {
            for (size_t j = 0; j < decomp_poly->d[i - 1] - decomp_poly->d[i]; ++j) f2lin_rng_generic_next_state(tmp);
            f2lin_rng_generic_add(tmp, h[decomp_poly->h[i]]);
        }

        for (size_t i = 0; i < decomp_poly->d[decomp_poly->m - 1]; ++i) f2lin_rng_generic_next_state(tmp);
    }

    f2lin_rng_generic_add(tmp, h[decomp_poly->hm1]);
    f2lin_rng_generic_add(tmp, h[0]);
    f2lin_rng_generic_copy(rng, tmp);

    f2lin_rng_generic_destroy(tmp);

    return rng;
}
