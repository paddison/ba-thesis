#include "rng_generic.h"

#ifndef CALC_MIN_POLY
#include "minpoly64.h"
#endif

#define XOR64_RNG_STATE_SIZE 64

typedef struct F2LinRngGeneric F2LinRngGeneric;

struct F2LinRngGeneric {
    uint64_t state;
};

const static uint64_t a = 13;
const static uint64_t b = 17;
const static uint64_t c = 5;

F2LinRngGeneric* f2lin_rng_generic_init_zero() {
    return (F2LinRngGeneric*) calloc(1, sizeof(F2LinRngGeneric));
}

F2LinRngGeneric* f2lin_rng_generic_init() {
    F2LinRngGeneric* rng = (F2LinRngGeneric* )calloc(1, sizeof(F2LinRngGeneric));
    rng->state = 12323456ull;
    return rng;
}

F2LinRngGeneric* f2lin_rng_generic_init_seed(uint64_t seed) {
    F2LinRngGeneric* rng = f2lin_rng_generic_init_zero();
    rng->state = seed;
    return rng;
}

F2LinRngGeneric* f2lin_rng_generic_copy(F2LinRngGeneric *dest, 
        const F2LinRngGeneric *source) {
    dest->state = source->state;
    return dest;
}

F2LinRngGeneric* f2lin_rng_generic_add(F2LinRngGeneric* lhs, 
        const F2LinRngGeneric* rhs) {
    lhs->state ^= rhs->state;
    return lhs;
}

uint64_t f2lin_rng_generic_next_state(F2LinRngGeneric* rng) {
    rng->state ^= (rng->state << a);
    rng->state ^= (rng->state >> b);
    rng->state ^= (rng->state << c);
    return rng->state;
}

uint64_t f2lin_rng_generic_gen64(F2LinRngGeneric* rng) {
    return f2lin_rng_generic_next_state(rng);
}

void f2lin_rng_generic_gen_n_numbers(F2LinRngGeneric* rng, size_t N, uint64_t buf[N]) {
    for (size_t i = 0; i < N; ++i) buf[i] = f2lin_rng_generic_gen64(rng);
}

long f2lin_rng_generic_state_size() {
    return XOR64_RNG_STATE_SIZE;
}

void f2lin_rng_generic_destroy(F2LinRngGeneric* rng) {
    free(rng);
}

#ifndef CALC_MIN_POLY
char* f2lin_rng_generic_min_poly() {
    return MIN_POLY;
}
#endif

int f2lin_rng_generic_compare_state(F2LinRngGeneric* lhs, F2LinRngGeneric* rhs) {
    return lhs->state == rhs->state;  
}
