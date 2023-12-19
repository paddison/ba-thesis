#include "rng_generic.h"

#ifndef CALC_MIN_POLY
#include "minpolyxoshiro.h"
#endif

#define XOR64_RNG_STATE_SIZE 256

typedef struct Xor64RngGeneric Xor64RngGeneric;

struct Xor64RngGeneric {
    uint64_t state[4];
};

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

Xor64RngGeneric* xor64_rng_generic_init_zero() {
    return (Xor64RngGeneric*) calloc(1, sizeof(Xor64RngGeneric));
}

Xor64RngGeneric* xor64_rng_generic_init() {
    Xor64RngGeneric* rng = (Xor64RngGeneric* )calloc(1, sizeof(Xor64RngGeneric));
    rng->state[0] = 12323456ull;
    rng->state[1] = 54321ull;
    rng->state[2] = 0xa02b4c7dull;
    rng->state[3] = 0x94a3f7eeull;
    return rng;
}

Xor64RngGeneric* xor64_rng_generic_init_seed(uint64_t seed) {
    Xor64RngGeneric* rng = xor64_rng_generic_init_zero();
    rng->state[0] = seed;
    // use splitmix?
    return rng;
}

Xor64RngGeneric* xor64_rng_generic_copy(Xor64RngGeneric *dest, 
        const Xor64RngGeneric *source) {
    dest->state[0] = source->state[0];
    dest->state[1] = source->state[1];
    dest->state[2] = source->state[2];
    dest->state[3] = source->state[3];
    return dest;
}

Xor64RngGeneric* xor64_rng_generic_add(Xor64RngGeneric* lhs, 
        const Xor64RngGeneric* rhs) {
    lhs->state[0] ^= rhs->state[0];
    lhs->state[1] ^= rhs->state[1];
    lhs->state[2] ^= rhs->state[2];
    lhs->state[3] ^= rhs->state[3];
    return lhs;
}

uint64_t xor64_rng_generic_next_state(Xor64RngGeneric* rng) {
    uint64_t* s = rng->state;
	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

    return s[3];
}

uint64_t xor64_rng_generic_gen64(Xor64RngGeneric* rng) {
    uint64_t* s = rng->state;
    const uint64_t result = rotl(s[0] + s[3], 23) + s[0];
    xor64_rng_generic_next_state(rng);
    return result;
}

void xor64_rng_generic_gen_n_numbers(Xor64RngGeneric* rng, size_t N, uint64_t buf[N]) {
    for (size_t i = 0; i < N; ++i) buf[i] = xor64_rng_generic_gen64(rng);
}

long xor64_rng_generic_state_size() {
    return XOR64_RNG_STATE_SIZE;
}

void xor64_rng_generic_destroy(Xor64RngGeneric* rng) {
    free(rng);
}

#ifndef CALC_MIN_POLY
char* xor64_rng_generic_min_poly() {
    return MIN_POLY;
}
#endif

#ifdef TEST
int xor64_rng_generic_compare_state(Xor64RngGeneric* lhs, Xor64RngGeneric* rhs) {
    return lhs->state[0] == rhs->state[0] &&
           lhs->state[1] == rhs->state[1] && 
           lhs->state[2] == rhs->state[2] &&   
           lhs->state[3] == rhs->state[3]; 
}
#endif
