#include "NTL/GF2X.h"
#include "xorshift64.h"
#include "c_poly.h"
#include "prepare_jump.hpp"

NTL::GF2X jump_poly;
NTL::GF2X minimal_poly;

void _init_min_poly(uint64_t seed);
void _init_jump_poly(uint64_t jump_size);
PolyGF2 _gf2x_to_c_poly_gf2(NTL::GF2X jp);

/**************************
 * Header Implementations *
 **************************/

extern "C" PolyGF2 init_jump(unsigned long long jump_size) {
    _init_min_poly(362436000ull);
    _init_jump_poly(jump_size);
    return _gf2x_to_c_poly_gf2(jump_poly);
}


/*****************************************
 * Functions for internal implementation *
 *****************************************/

PolyGF2 _gf2x_to_c_poly_gf2(NTL::GF2X jp) {
    PolyGF2 ret;
    poly_gf2_init(&ret, NTL::deg(jp));

    for (size_t i = 0; i <= NTL::deg(jp); ++i) {
        if (NTL::coeff(jp, i) == 1)  {
            ret.p[i / (sizeof(PolyRaw) * 8)] |= 1ull << i;
        }
    }

    return ret;
}

void _init_min_poly(uint64_t seed) {
    const size_t seq_len = 2 * sizeof(StateRng) * 8;
    NTL::vec_GF2 seq(NTL::INIT_SIZE, seq_len);

    StateRng rng = (StateRng) { .s = seed };

    for (size_t i = 0; i < seq_len; ++i) {
        step(&rng); 
        seq[i] = rng.s & 0x01ul;
    }

    NTL::MinPolySeq(minimal_poly, seq, seq_len);
}

void _init_jump_poly(uint64_t jump_size) {
    NTL::GF2X x(NTL::INIT_MONO, 1);
    NTL::GF2XModulus minimal_poly_mod;

    NTL::build(minimal_poly_mod, minimal_poly);
    NTL::PowerMod(jump_poly, x, jump_size, minimal_poly_mod);
}
