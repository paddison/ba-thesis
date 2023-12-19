#include <stdio.h>

#include "rng_generic.h"
#include "lib/tinymt/tinymt.h"
#ifndef CALC_MIN_POLY 
#include "minpolytinymt.h"
#endif

struct Xor64RngGeneric {
    tinymt64_t tinymt64;
};

Xor64RngGeneric* xor64_rng_generic_init_zero() {

    Xor64RngGeneric* rng = (Xor64RngGeneric*) calloc(1, sizeof(Xor64RngGeneric));
    rng->tinymt64.mat1 = 0x65980cb3;
    rng->tinymt64.mat2 = 0xeb38facf;
    rng->tinymt64.tmat = 0xcc3b75ff;
    return rng;
}

Xor64RngGeneric* xor64_rng_generic_init() {
    Xor64RngGeneric* rng = xor64_rng_generic_init_zero();
    tinymt64_init(&rng->tinymt64, 2147482983);
    return rng;
}

Xor64RngGeneric* xor64_rng_generic_init_seed(uint64_t seed) {
    Xor64RngGeneric* rng = xor64_rng_generic_init_zero();
    tinymt64_init(&rng->tinymt64, seed);
    return rng;
    
}

Xor64RngGeneric* xor64_rng_generic_copy(Xor64RngGeneric* dest, const Xor64RngGeneric* source) {
    dest->tinymt64.status[0] = source->tinymt64.status[0];
    dest->tinymt64.status[1] = source->tinymt64.status[1];
    dest->tinymt64.mat1 = source->tinymt64.mat1;
    dest->tinymt64.mat2 = source->tinymt64.mat2;
    dest->tinymt64.tmat = source->tinymt64.tmat;

    return dest;
}
Xor64RngGeneric* xor64_rng_generic_add(Xor64RngGeneric* lhs, const Xor64RngGeneric* rhs) {
    lhs->tinymt64.status[0] ^= rhs->tinymt64.status[0];
    lhs->tinymt64.status[1] ^= rhs->tinymt64.status[1];

    return lhs;
}

uint64_t xor64_rng_generic_gen64(Xor64RngGeneric* rng) {
    return tinymt64_generate_uint64(&rng->tinymt64);
}

void xor64_rng_generic_gen_n_numbers(Xor64RngGeneric* rng, size_t N, uint64_t buf[N]) {
    for (size_t i = 0; i < N; ++i) buf[i] = xor64_rng_generic_gen64(rng);
}

uint64_t xor64_rng_generic_next_state(Xor64RngGeneric* rng) {
    tinymt64_next_state(&rng->tinymt64);
    return rng->tinymt64.status[0];
}

void xor64_rng_generic_destroy(Xor64RngGeneric* rng) {
    free(rng);
}

long xor64_rng_generic_state_size() {
    return 127;
}

#ifndef CALC_MIN_POLY
char* xor64_rng_generic_min_poly() {
    return MIN_POLY;
}
#endif

#ifdef TEST
int xor64_rng_generic_compare_state(Xor64RngGeneric* lhs, Xor64RngGeneric* rhs) {
    return lhs->tinymt64.status[0] == rhs->tinymt64.status[0] &&
           lhs->tinymt64.status[1] == rhs->tinymt64.status[1];
}
#endif
