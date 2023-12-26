#include <stdio.h>

#include "rng_generic.h"
#include "lib/tinymt/tinymt.h"
#ifndef CALC_MIN_POLY 
#include "minpolytinymt.h"
#endif

struct F2LinRngGeneric {
    tinymt64_t tinymt64;
};

F2LinRngGeneric* f2lin_rng_generic_init_zero() {

    F2LinRngGeneric* rng = (F2LinRngGeneric*) calloc(1, sizeof(F2LinRngGeneric));
    rng->tinymt64.mat1 = 0x65980cb3;
    rng->tinymt64.mat2 = 0xeb38facf;
    rng->tinymt64.tmat = 0xcc3b75ff;
    return rng;
}

F2LinRngGeneric* f2lin_rng_generic_init() {
    F2LinRngGeneric* rng = f2lin_rng_generic_init_zero();
    tinymt64_init(&rng->tinymt64, 2147482983);
    return rng;
}

F2LinRngGeneric* f2lin_rng_generic_init_seed(uint64_t seed) {
    F2LinRngGeneric* rng = f2lin_rng_generic_init_zero();
    tinymt64_init(&rng->tinymt64, seed);
    return rng;
    
}

F2LinRngGeneric* f2lin_rng_generic_copy(F2LinRngGeneric* dest, const F2LinRngGeneric* source) {
    dest->tinymt64.status[0] = source->tinymt64.status[0];
    dest->tinymt64.status[1] = source->tinymt64.status[1];
    dest->tinymt64.mat1 = source->tinymt64.mat1;
    dest->tinymt64.mat2 = source->tinymt64.mat2;
    dest->tinymt64.tmat = source->tinymt64.tmat;

    return dest;
}
F2LinRngGeneric* f2lin_rng_generic_add(F2LinRngGeneric* lhs, const F2LinRngGeneric* rhs) {
    lhs->tinymt64.status[0] ^= rhs->tinymt64.status[0];
    lhs->tinymt64.status[1] ^= rhs->tinymt64.status[1];

    return lhs;
}

uint64_t f2lin_rng_generic_gen64(F2LinRngGeneric* rng) {
    return tinymt64_generate_uint64(&rng->tinymt64);
}

void f2lin_rng_generic_gen_n_numbers(F2LinRngGeneric* rng, size_t N, uint64_t buf[N]) {
    for (size_t i = 0; i < N; ++i) buf[i] = f2lin_rng_generic_gen64(rng);
}

uint64_t f2lin_rng_generic_next_state(F2LinRngGeneric* rng) {
    tinymt64_next_state(&rng->tinymt64);
    return rng->tinymt64.status[0];
}

void f2lin_rng_generic_destroy(F2LinRngGeneric* rng) {
    free(rng);
}

long f2lin_rng_generic_state_size() {
    return 127;
}

#ifndef CALC_MIN_POLY
char* f2lin_rng_generic_min_poly() {
    return MIN_POLY;
}
#endif

int f2lin_rng_generic_compare_state(F2LinRngGeneric* lhs, F2LinRngGeneric* rhs) {
    return lhs->tinymt64.status[0] == rhs->tinymt64.status[0] &&
           lhs->tinymt64.status[1] == rhs->tinymt64.status[1];
}
