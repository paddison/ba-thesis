#include "xorshift64.h"

uint64_t a = 13;
uint64_t b = 17;
uint64_t c = 5;

StateRng64* state_rng_init(StateRng64* rng, void* seed) {
    uint64_t s = *((uint64_t*) (seed));
    rng->s = s;
    return rng;
}


StateRng64* state_rng_copy(StateRng64 *dest, const StateRng64 *source) {
    dest->s = source->s;
    return dest;
}

StateRng64* state_rng_add(StateRng64* lhs, const StateRng64* rhs) {
    lhs->s ^= rhs->s;
    return lhs;
}

StateRng64* step(StateRng64* rng) {
    rng->s ^= (rng->s << a);
    rng->s ^= (rng->s >> b);
    rng->s ^= (rng->s << c);
    return rng;
}

void set_params(uint64_t aa, uint64_t bb, uint64_t cc) {
    a = aa;
    b = bb;
    c = cc;
}
