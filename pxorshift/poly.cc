#include "NTL/GF2X.h"
#include "NTL/tools.h"
#include "NTL/vec_GF2.h"
#include "gray/gray.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

#define SEED 123234345ull
#define Q 8

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

int horner_man(size_t deg, int poly[deg], int x) {
    size_t i = 0;
    while (poly[i] == 0) ++i;

    int ret = x * poly[i];
    ++i;

    printf("%d\n", ret);
    for (; i < deg - 1; ++i) {
        if (poly[i] != 0) ret += poly[i];
        ret *= x; 
        printf("%d\n", ret);
    }
    if (poly[deg - 1] != 0) ret += poly[deg - 1];
    printf("%d\n", ret);
    return ret;
}

uint16_t gray[1 << Q];

uint64_t h[1 << Q];
uint16_t t[1 << Q];

void print_gray(size_t i) {
    uint64_t mask = 1 << (Q - 1);
    while (mask > 0) {
        std::cout << ((gray[i] & mask) != 0 ? 1 : 0) << " ";
        mask /= 2;
    }
    std::cout << std::endl;
}

void gray_enumeration() {
    size_t q = 4;
    uint64_t A_j[Q + 1];
    uint64_t seed = 3;
    uint64_t seed_tmp = seed;
    gray_code(Q, gray);

    // precompute A^j * x 
    for (size_t i = 0; i < Q + 1; ++i) { 
        A_j[i] = seed_tmp;
        seed_tmp <<= 2;
    }

    for (size_t i = 0; i < Q + 1; ++i) {
        std::cout << A_j[i] << std::endl; 
    }
    // do all the rest
    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    h[0] = A_j[Q];
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        uint64_t diff = gray[i] ^ gray[i - 1];
        size_t set_bit = 0;
        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }
        print_gray(i);

        h[gray[i]] = h[gray[i - 1]] ^ A_j[set_bit - 1];
        std::bitset<16> bin(h[gray[i]]);
        std::cout << "h(A)*x: " << bin << " diff: " << set_bit - 1 << std::endl;

    }

    
}

uint64_t sliding_window(uint64_t seed) {
    // in the paper, the seed is denoted as 'x'

    uint64_t A_j[Q + 1]; // = A^j * x
    uint64_t seed_tmp = seed;
    
    /*
    g = NTL::GF2X();

    NTL::SetCoeff(g, 18, 0);
    NTL::SetCoeff(g, 17, 0);
    NTL::SetCoeff(g, 16, 0);
    NTL::SetCoeff(g, 15, 1);
    NTL::SetCoeff(g, 14, 1);
    NTL::SetCoeff(g, 13, 1);
    NTL::SetCoeff(g, 12, 1);
    NTL::SetCoeff(g, 11, 0);
    NTL::SetCoeff(g, 10, 1);
    NTL::SetCoeff(g, 9, 0);
    NTL::SetCoeff(g, 8, 0);
    NTL::SetCoeff(g, 7, 0);
    NTL::SetCoeff(g, 6, 1);
    NTL::SetCoeff(g, 5, 1);
    NTL::SetCoeff(g, 4, 1);
    NTL::SetCoeff(g, 3, 0);
    NTL::SetCoeff(g, 2, 0);
    NTL::SetCoeff(g, 1, 0);
    NTL::SetCoeff(g, 0, 1);

    std::cout << g << std::endl;
    std::cout << NTL::deg(g) << std::endl;
    
    */
    gray_code(Q, gray);

    // precompute A^j * x 
    for (size_t i = 0; i < Q + 1; ++i) { 
        A_j[i] = seed_tmp;
        //std::cout << A_j[i] << " " << seed_tmp << std::endl;
        seed_tmp = step(seed_tmp);
    }
    // do all the rest
    // polynomials in h always contain z^q, so 
    // each of the 2^q polynomials h contains A^j * x
    h[0] = A_j[Q];
    
    for (size_t i = 1; i < (1 << Q); ++i) {
        uint64_t diff = gray[i] ^ gray[i - 1];
        //std::cout << i << std::endl;
        size_t set_bit = 0;
        while (diff > 0) {
            diff >>= 1;
            ++set_bit;
        }
        h[gray[i]] = h[gray[i - 1]] ^ A_j[set_bit - 1];
    }

    // use horners method with sliding window
    uint64_t state = 0;
    int deg = NTL::deg(g);
    int i = deg; 
    size_t coeffs_id = 0;
    int d1 = 0;
    int d2;

    // go to first non zero coefficient
    while (g[i] == 0) --i;
    d1 = i;

    // determine which gray enumeration to use
    for (size_t j = 0; j < Q; ++j) {
        coeffs_id = (coeffs_id << 1) ^ NTL::rep(NTL::coeff(g, i - j - 1));
    }

    // h1(A) * x

#define w 10
    std::bitset<w> gray(coeffs_id);
    state = h[coeffs_id];
    //std::cout << "h1: " << gray << ", d1: " << d1 << ", d2: " << d2 << std::endl;
    i -= (Q + 1);

    for (; i >= Q; --i) {
        if (g[i] == 1) {
            d2 = i;
            // std::cout << "A^" << d1 - d2 << std::endl;
            for (size_t j = 0; j < d1 - d2; ++j) state = step(state);

            coeffs_id = 0;  

            for (size_t j = 0; j < Q; ++j) {
                coeffs_id = (coeffs_id << 1) ^ NTL::rep(NTL::coeff(g, i - j - 1));
            }

            std::bitset<w> gray(coeffs_id);
            //std::cout << "xor with: " << gray << ", d1: " << d1 << ", d2: " << d2 << std::endl;
            state ^= h[coeffs_id];
            i -= Q;
            d1 = d2;
        }
    }
    coeffs_id = 0;
    int dm = i + 1;

    //std::cout << "A^" << dm << std::endl;
    for (size_t j = 0; j < dm; ++j) state = step(state);
    while (i > -1) {
        if (NTL::coeff(g, i) == 1) { 
            coeffs_id |= (NTL::rep(NTL::coeff(g, i)) & 1) << i;
        }
        --i;
    }
    std::bitset<w> gray2(coeffs_id);
    state ^= h[coeffs_id];

    //std::cout << "dm: " << dm << " hm+1: " << gray2 << std::endl;
    
    state ^= h[0];

    return state;
}


// calculate jump polynomial by evaluating with horners method
int main(void) {
    size_t J = 123456;
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
