#include "NTL/GF2X.h"
#include "NTL/tools.h"
#include "NTL/vec_GF2.h"
#include "gray/gray.h"
#include <bitset>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cassert>

#define SEED 123234345ull
#define Q 10

const size_t STATE_SIZE = 64;
const uint64_t a = 698769069ull;
uint64_t c = 7654321;

// minimal polynomial p
NTL::GF2X p;
// jump polynomial g
NTL::GF2X g;

uint64_t step(uint64_t s) {
    s ^= (s << 13);
    s ^= (s >> 17);
    s ^= (s << 5);
    return s;
}

void init_min_poly(uint64_t seed) {
    const size_t SEQ_LEN = 2 * STATE_SIZE;
    NTL::vec_GF2 seq(NTL::INIT_SIZE, SEQ_LEN);

    for (size_t i = 0; i < SEQ_LEN; ++i) {
        seed = step(seed); 
        seq[i] = seed & 0x01ul;
    }

    NTL::MinPolySeq(p, seq, STATE_SIZE);
}

void init_jump_poly(long jump_size) {
    NTL::GF2X x(NTL::INIT_MONO, jump_size);
    g = x % p;
}

// calculate jump polynomial by evaluating with horners method
uint64_t horner(uint64_t x) {
    uint64_t tmp_state = x;
    size_t deg = NTL::deg(g);
    int i = deg;

    while (NTL::coeff(g, i) == 0) --i;


    if (i > 0) {
        tmp_state = step(tmp_state);
        --i;
        for(; i > 0; --i) {
            if(NTL::coeff(g, i) != 0) tmp_state ^= x;
            tmp_state = step(tmp_state);
        }

        if(NTL::coeff(g, 0) != 0) tmp_state ^= x;
    }

    return tmp_state;
}

uint32_t jump(uint64_t seed) {
    return horner(seed);    
}

uint16_t gray[1 << Q];

uint64_t h[1 << Q];
uint16_t t[1 << Q];

void print_gray(size_t i) {
    uint64_t mask = 1 << (Q - 1);
    std::bitset<Q> bits(i);
    std::cout << bits << std::endl;
}

// represents the current state, initialized with the seed
struct State {
    uint64_t s;
};

// precomputes A^0x..A^qx which are the first q state transitions
// (equivalent to repeated calls to step()
uint64_t* precompute_A(size_t q, uint64_t A[q + 1], const State s) {
    State state_tmp = s;
    for (size_t i = 0; i < Q + 1; ++i) { 
        A[i] = state_tmp.s;
        //std::cout << A_j[i] << " " << seed_tmp << std::endl;
        state_tmp.s = step(state_tmp.s);
    }
    
    return A;
}

uint64_t* compute_decomposition_polys(size_t q, uint64_t h[1 << q], 
                                      uint64_t A[q + 1]) {
    // A[Q] is the polynomial component which contains z^q,
    // which is always included, and thus used to initialize
    // the decomposition polys
    h[0] = A[Q];
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        // find out which bit flips
        uint64_t diff = gray[i] ^ gray[i - 1];
        size_t set_bit = 0;

        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }

        h[gray[i]] = h[gray[i - 1]] ^ A[set_bit - 1];
    }

    return h;
}

inline size_t determine_gray_enumeration(size_t q, size_t i, NTL::GF2X poly);

uint64_t horner_sliding_window(size_t q, uint64_t h[1 << q]) {
    // use horners method with sliding window
    uint64_t state = 0;
    int i = NTL::deg(g); 
    std::cout << "poly_length: " << i << std::endl;
    int dj = 0, dj_next = 0, dm = -1;

    // go to first non zero coefficient
    // NTL polynomials should always start with the first non zero component
    assert(NTL::coeff(g, i) == 1);
    dj = i;

    // determine which gray enumeration to use
    
    //; h1(A) * x, first component in horner's method
    size_t gray = determine_gray_enumeration(Q, i, g);
    state = h[gray];//[determine_gray_enumeration(Q, i, g)];

    i -= (Q + 1);

    for (; i >= Q; --i) {
        if (g[i] == 1) {
            std::cout << "i: " << i << std::endl;
            dj_next = i;
            // step forward dj - dj_next steps, which is the same as
            // multiplying with A^(dj - dj_next)
            for (size_t j = 0; j < dj - dj_next; ++j) state = step(state);

            // find out the gray_enumeration of the current decomposition
            // polynomials and calculate cur_state + h_i(A)x
            size_t gray = determine_gray_enumeration(Q, i, g);
            state ^= h[gray];//determine_gray_enumeration(Q, i, g)];

            i -= Q;
            dj = dj_next;
        }
    }

    // calculate h_m+1 and do the last part of the decomposition 
    // which is multiplying the current state with A^dm
    // xoring with h_m+1(A)x and adding A^qx
    dm = dj - Q;

    for (size_t j = 0; j < dm; ++j) state = step(state);

    size_t g2 = determine_gray_enumeration(i + 1, i + 1, g);
    return state ^ h[g2] ^ h[0];
}

inline size_t determine_gray_enumeration(size_t q, size_t i, NTL::GF2X poly) {
    size_t gray_enumeration = 0;

    for (size_t j = 0; j < q; ++j) {
        gray_enumeration = (gray_enumeration << 1) ^ NTL::rep(NTL::coeff(g, i - j - 1));
    }
    
    return gray_enumeration;
}

uint64_t sliding_window(uint64_t seed) {
    // in the paper, the seed is denoted as 'x'

    uint64_t A[Q + 1]; // = A^j * x
    uint64_t seed_tmp = seed;
    State s = (State) { .s = seed };
    
    // do gray code enumeration
    gray_code(Q, gray);

    // precompute A^j * x 
    precompute_A(Q, A, s);

    // do all the rest
    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    compute_decomposition_polys(Q, h, A);

    return horner_sliding_window(Q, h);
}


// calculate jump polynomial by evaluating with horners method
int main(void) {
    size_t J = 123456000;
    uint64_t seed = SEED;
    init_min_poly(seed);
    init_jump_poly(J);
    uint64_t seq = seed;
    std::cout << p << std::endl;
    std::cout << g << std::endl;

    for (size_t i = 0; i < J; ++i) {
        seq = step(seq);
    }
    std::cout << seq << "\n";
    uint64_t j = horner(seed);
    std::cout << j << "\n";

    uint64_t j2 = sliding_window(seed);
    std::cout << j2 << "\n";
    //gray_enumeration();
    return EXIT_SUCCESS;
}
