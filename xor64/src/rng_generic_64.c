#include "rng_generic.h"

#ifndef CALC_MIN_POLY
#include "minpoly64.h"
#endif

#define XOR64_RNG_STATE_SIZE 64

typedef struct Xor64RngGeneric Xor64RngGeneric;

struct Xor64RngGeneric {
    uint64_t state;
};

const static uint64_t a = 13;
const static uint64_t b = 17;
const static uint64_t c = 5;

Xor64RngGeneric* xor64_rng_generic_init_zero() {
    return (Xor64RngGeneric*) calloc(1, sizeof(Xor64RngGeneric));
}

Xor64RngGeneric* xor64_rng_generic_init() {
    Xor64RngGeneric* rng = (Xor64RngGeneric* )calloc(1, sizeof(Xor64RngGeneric));
    rng->state = 12323456ull;
    return rng;
}

Xor64RngGeneric* xor64_rng_generic_copy(Xor64RngGeneric *dest, 
        const Xor64RngGeneric *source) {
    dest->state = source->state;
    return dest;
}

Xor64RngGeneric* xor64_rng_generic_add(Xor64RngGeneric* lhs, 
        const Xor64RngGeneric* rhs) {
    lhs->state ^= rhs->state;
    return lhs;
}

void xor64_rng_generic_next_state(Xor64RngGeneric* rng) {
    rng->state ^= (rng->state << a);
    rng->state ^= (rng->state >> b);
    rng->state ^= (rng->state << c);
}

uint64_t xor64_rng_generic_gen64(Xor64RngGeneric* rng) {
    xor64_rng_generic_next_state(rng);
    return rng->state;
}

Xor64RngGeneric* xor64_rng_generic_init_seed(uint64_t seed) {
    Xor64RngGeneric* rng = xor64_rng_generic_init_zero();
    rng->state = seed;
    return rng;
}

#ifndef CALC_MIN_POLY
char* xor64_rng_generic_min_poly() {
    return MIN_POLY;
}
#endif

long xor64_rng_generic_state_size() {
    return XOR64_RNG_STATE_SIZE;
}

void xor64_rng_generic_destroy(Xor64RngGeneric* rng) {
    free(rng);
}

#ifdef TEST
int xor64_rng_generic_compare_state(Xor64RngGeneric* lhs, Xor64RngGeneric* rhs) {
    return lhs->state == rhs->state;  
}
#endif
