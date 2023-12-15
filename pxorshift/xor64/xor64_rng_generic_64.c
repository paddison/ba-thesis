#include "xor64_rng_generic.h"

static uint64_t a = 13;
static uint64_t b = 17;
static uint64_t c = 5;

Xor64RngGeneric* xor64_rng_generic_init(Xor64RngGeneric* rng) {
    rng->state = 12323456ull;
    return rng;
}


Xor64RngGeneric* xor64_rng_generic_copy(Xor64RngGeneric *dest, const Xor64RngGeneric *source) {
    dest->state = source->state;
    return dest;
}

Xor64RngGeneric* xor64_rng_generic_add(Xor64RngGeneric* lhs, const Xor64RngGeneric* rhs) {
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

Xor64RngGeneric* xor64_rng_generic_seed(Xor64RngGeneric* rng, uint64_t seed) {
    rng->state = seed;
    return rng;
}

static void set_params(uint64_t aa, uint64_t bb, uint64_t cc) {
    a = aa;
    b = bb;
    c = cc;
}


