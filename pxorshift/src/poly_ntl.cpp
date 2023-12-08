#include "NTL/GF2X.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cassert>
#include "poly_ntl.hpp"

#define SEED 123234345ull
#define Q 10

size_t jump_size;
//size_t q = 4;

NTL::GF2X jump_poly;
NTL::GF2X minimal_poly;

StateRng h[1 << Q];
uint16_t gray[1 << Q];

JumpAlgorithm algo = POLY_SLIDING_WINDOW;

const size_t STATE_SIZE = sizeof(StateRng) * 8;

void init_min_poly(uint64_t seed) {
    std::cout << STATE_SIZE << std::endl;
    const size_t SEQ_LEN = 2 * STATE_SIZE;
    NTL::vec_GF2 seq(NTL::INIT_SIZE, SEQ_LEN);

    StateRng rng = (StateRng) { .s = seed };

    for (size_t i = 0; i < SEQ_LEN; ++i) {
        step(&rng); 
        seq[i] = rng.s & 0x01ul;
    }

    NTL::MinPolySeq(minimal_poly, seq, STATE_SIZE);
}


void init_jump_poly(uint64_t jump_size) {
    NTL::GF2X x(NTL::INIT_MONO, 1);
    NTL::GF2XModulus minimal_poly_mod;

    NTL::build(minimal_poly_mod, minimal_poly);
    NTL::PowerMod(jump_poly, x, jump_size, minimal_poly_mod);
}


// calculate jump polynomial by evaluating with horners method
StateRng* horner(StateRng* rng) {
    StateRng tmp = { 0 }; 
    size_t i = NTL::deg(jump_poly);

    state_rng_copy(&tmp, rng);

    while (NTL::coeff(jump_poly, i) == 0) --i;

    if (i > 0) {
        step(&tmp);
        --i;
        for(; i > 0; --i) {
            if(NTL::coeff(jump_poly, i) != 0) state_rng_add(&tmp, rng);
            step(&tmp);
        }

        if(NTL::coeff(jump_poly, 0) != 0) state_rng_add(&tmp, rng);
    }

    *rng = tmp;
    return rng;
}

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

inline size_t determine_gray_enumeration(size_t q, size_t i, NTL::GF2X poly);

StateRng* horner_sliding_window(StateRng* rng, size_t q, StateRng h[1 << q]) {
    // use horners method with sliding window
    StateRng tmp = { 0 };
    int i = NTL::deg(jump_poly); 
    std::cout << "poly_length: " << i << std::endl;
    int dj = 0, dj_next = 0, dm = -1;

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component
    assert(NTL::coeff(jump_poly, i) == 1);
    dj = i;

    //; h1(A) * x, first component in horner's method
    state_rng_copy(&tmp, &h[determine_gray_enumeration(Q, i, jump_poly)]);

    i -= (Q + 1);

    for (; i >= Q; --i) {
        if (NTL::coeff(jump_poly, i) == 1) {
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

inline size_t determine_gray_enumeration(size_t q, size_t i, NTL::GF2X poly) {
    size_t gray_enumeration = 0;

    for (size_t j = 0; j < q; ++j) {
        gray_enumeration = (gray_enumeration << 1) ^ NTL::rep(NTL::coeff(poly, i - j - 1));
    }
    
    return gray_enumeration;
}

StateRng* sliding_window(StateRng* rng) {
    // in the paper, the seed is denoted as 'x'
    StateRng A[Q + 1]; // = A^j * x

    // do gray code enumeration
    gray_code(Q, gray);

    // precompute A^j * x 
    precompute_A(Q, A, rng);

    // do all the rest
    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    compute_decomposition_polys(Q, h, A);

    return horner_sliding_window(rng, Q, h);
}

// calculate jump polynomial by evaluating with horners method
int main(void) {
    size_t J = 123456;
    StateRng rng = (StateRng) { .s = SEED };
    StateRng rng_sliding = (StateRng) { .s = SEED };
    init_min_poly(SEED);
    init_jump_poly(J);
    StateRng seq = (StateRng) { .s = SEED };
    std::cout << minimal_poly << std::endl;
    std::cout << jump_poly << std::endl;

    for (size_t i = 0; i < J; ++i) {
        step(&seq);
    }
    std::cout << seq.s << "\n";
    uint64_t j = horner(&rng)->s;
    std::cout << j << "\n";

    uint64_t j2 = sliding_window(&rng_sliding)->s;
    std::cout << j2 << "\n";
    //gray_enumeration();
    return EXIT_SUCCESS;
}
