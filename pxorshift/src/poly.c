#include "gray/gray.h"
#include "poly.h"
#include "prepare_jump.hpp"
#include "c_poly.h"

#define SEED 123234345ull
#define Q 10

size_t jump_size;
//size_t q = 4;

PolyGF2 jump_poly;
PolyGF2Decomp decomp_poly;

StateRng h[1 << Q];
uint16_t gray[1 << Q];

enum JumpAlgorithm algo = SLIDING_WINDOW;

const size_t STATE_SIZE = sizeof(StateRng) * 8;

/*******************
 * Horner's method *
 *******************/

/* Forward Declarations */
StateRng* horner(StateRng* rng);


/*************************
 * Sliding window method *
 *************************/

/* Forward declarations */
/* Sliding window method, which acts directly on the polynomial */
StateRng* sliding_window(StateRng* rng);
/* Sliding window method, implemented on decomposition polynomial */
StateRng* sliding_window_decomp(StateRng* rng);

StateRng* horner_sliding_window(StateRng* rng, size_t q, StateRng h[1 << q]);
StateRng* horner_sliding_window_decomp(StateRng* rng, size_t q, StateRng h[1 << q]);

/* Sets up data needed for sliding window */
void prepare_sliding_window(StateRng* rng);

inline size_t determine_gray_enumeration(size_t q, size_t i, PolyGF2 poly);
StateRng* precompute_A(size_t q, StateRng A[q + 1], const StateRng* rng);
StateRng* compute_decomposition_polys(size_t q, StateRng h[1 << q], StateRng A[q + 1]);

void init(size_t jump_size) {
    jump_poly = prepare_jump_poly(jump_size);
    if (algo == SLIDING_WINDOW_DECOMP) {
        decomp_poly = poly_gf2_decomp(&jump_poly, Q);
    }
}

void set_jump_algorithm(enum JumpAlgorithm algorithm) {
    algo = algorithm;
}

StateRng* jump(StateRng* rng) {
    switch (algo) {
        case HORNER: return horner(rng); break;
        case SLIDING_WINDOW: return sliding_window(rng); break;
        case SLIDING_WINDOW_DECOMP: return sliding_window_decomp(rng); break;
    }
}

/*
 * Implementations
 */

// calculate jump polynomial by evaluating with horners method
StateRng* horner(StateRng* rng) {
    StateRng tmp = { 0 }; 
    size_t i = poly_gf2_deg(&jump_poly);

    state_rng_copy(&tmp, rng);

    while (poly_gf2_coeff(&jump_poly, i) == 0) --i;

    if (i > 0) {
        step(&tmp);
        --i;
        for(; i > 0; --i) {
            if(poly_gf2_coeff(&jump_poly, i) != 0) state_rng_add(&tmp, rng);
            step(&tmp);
        }

        if(poly_gf2_coeff(&jump_poly, 0) != 0) state_rng_add(&tmp, rng);
    }

    *rng = tmp;
    return rng;
}



/*
 * Implementations
 */

// precomputes A^0x..A^qx which are the first q state transitions
// (equivalent to repeated calls to step()
StateRng* precompute_A(size_t q, StateRng A[q + 1], const StateRng* rng) {
    StateRng tmp; 
    state_rng_copy(&tmp, rng);

    for (size_t i = 0; i < Q + 1; ++i) { 
        A[i] = tmp;
        step(&tmp);
    }
    
    return A;
}

StateRng* compute_decomposition_polys(size_t q, StateRng h[1 << q], 
                                      StateRng A[q + 1]) {
    // A[Q] is the polynomial component which contains z^q,
    // which is always included, and thus used to initialize
    // the decomposition polys
    state_rng_copy(&h[0], &A[Q]);
    h[0] = A[Q];
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        // find out which bit flips
        uint64_t diff = gray[i] ^ gray[i - 1];
        size_t set_bit = 0;

        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }

        state_rng_copy(&h[gray[i]], &h[gray[i - 1]]);
        state_rng_add(&h[gray[i]], &A[set_bit - 1]);
    }

    return h;
}

StateRng* horner_sliding_window(StateRng* rng, size_t q, StateRng h[1 << q]) {
    // use horners method with sliding window
    StateRng tmp = { 0 };
    int i = poly_gf2_deg(&jump_poly); 
    int dj = 0, dj_next = 0, dm = -1;

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component
    dj = i;

    //; h1(A) * x, first component in horner's method
    state_rng_copy(&tmp, &h[determine_gray_enumeration(Q, i, jump_poly)]);

    i -= (Q + 1);

    for (; i >= Q; --i) {
        if (poly_gf2_coeff(&jump_poly, i) == 1) {
            dj_next = i;
            // step forward dj - dj_next steps, which is the same as
            // multiplying with A^(dj - dj_next)
            for (size_t j = 0; j < dj - dj_next; ++j) step(&tmp);

            // find out the gray_enumeration of the current decomposition
            // polynomials and calculate cur_state + h_i(A)x
            state_rng_add(&tmp, &h[determine_gray_enumeration(Q, i, jump_poly)]);

            i -= Q;
            dj = dj_next;
        }
    }

    // calculate h_m+1 and do the last part of the decomposition 
    // which is multiplying the current state with A^dm
    // xoring with h_m+1(A)x and adding A^qx
    dm = dj - Q;

    for (size_t j = 0; j < dm; ++j) step(&tmp);

    state_rng_add(&tmp, &h[determine_gray_enumeration(i + 1, i + 1, jump_poly)]);
    state_rng_add(&tmp, &h[0]);

    state_rng_copy(rng, &tmp);
    return rng;
}

inline size_t determine_gray_enumeration(size_t q, size_t i, PolyGF2 poly) {
    size_t gray_enumeration = 0;

    for (size_t j = 0; j < q; ++j) {
        gray_enumeration = (gray_enumeration << 1) ^ poly_gf2_coeff(&poly, i - j - 1);
    }
    
    return gray_enumeration;
}

void prepare_sliding_window(StateRng* rng) {
    // in the paper, the seed is denoted as 'x'
    StateRng A[Q + 1];

    // do gray code enumeration
    gray_code(Q, gray);

    // precompute A^j * x 
    precompute_A(Q, A, rng);

    // do all the rest
    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    compute_decomposition_polys(Q, h, A);
}

StateRng* sliding_window(StateRng* rng) {
    prepare_sliding_window(rng);

    return horner_sliding_window(rng, Q, h);
}


StateRng* horner_sliding_window_decomp(StateRng* rng, size_t q, StateRng h[1 << q]) {
    StateRng tmp = { 0 };

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component

    //; h1(A) * x, first component in horner's method
    size_t dj = decomp_poly.params[0].d, dj_next;
    state_rng_copy(&tmp, &h[decomp_poly.params[0].h]);


    for (size_t i = 1; i < decomp_poly.m; ++i) {
        dj_next = decomp_poly.params[i].d;
        for (size_t j = 0; j < dj - dj_next; ++j) step(&tmp);
        state_rng_add(&tmp, &h[decomp_poly.params[1].h]);
    }

    for (size_t i = 0; i < decomp_poly.params[decomp_poly.m - 1].d; ++i) step(&tmp);

    state_rng_add(&tmp, &h[decomp_poly.hm1]);
    state_rng_add(&tmp, &h[0]);
    state_rng_copy(rng, &tmp);

    return rng;
}

StateRng* sliding_window_decomp(StateRng* rng) {
    prepare_sliding_window(rng);
    return horner_sliding_window_decomp(rng, Q, h);
}

// calculate jump polynomial by evaluating with horners method
/*
int main(void) {
    size_t J = 123456;
    jump_poly = init_jump(J);
    StateRng rng = (StateRng) { .s = SEED };
    StateRng rng_sliding = (StateRng) { .s = SEED };
    PolyGF2 pp = init_jump(J);
    StateRng seq = (StateRng) { .s = SEED };

    for (size_t i = 0; i < J; ++i) {
        step(&seq);
    }
    uint64_t j = horner(&rng)->s;

    uint64_t j2 = sliding_window(&rng_sliding)->s;
    //gray_enumeration();
    return EXIT_SUCCESS;
}
*/
