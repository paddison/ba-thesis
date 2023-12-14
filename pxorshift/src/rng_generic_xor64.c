#include "rng_generic_xor64.h"

static uint64_t a = 13;
static uint64_t b = 17;
static uint64_t c = 5;

RngGeneric* rng_generic_init(RngGeneric* rng) {
    rng->s = 12323456ull;
    return rng;
}

RngGeneric* rng_generic_copy(RngGeneric *dest, const RngGeneric *source) {
    dest->s = source->s;
    return dest;
}

RngGeneric* rng_generic_add(RngGeneric* lhs, const RngGeneric* rhs) {
    lhs->s ^= rhs->s;
    return lhs;
}

uint64_t rng_generic_step(RngGeneric* rng) {
    rng->s ^= (rng->s << a);
    rng->s ^= (rng->s >> b);
    rng->s ^= (rng->s << c);
    return  rng->s;
}

void set_params(uint64_t aa, uint64_t bb, uint64_t cc) {
    a = aa;
    b = bb;
    c = cc;
}
