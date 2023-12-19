#include "rng_generic.h"

#ifndef CALC_MIN_POLY
#include "minpolyxoshiro.h"
#endif

#define XOR64_RNG_STATE_SIZE 256

typedef struct F2LinRngGeneric F2LinRngGeneric;

struct F2LinRngGeneric {
    uint64_t state[4];
};

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

F2LinRngGeneric* f2lin_rng_generic_init_zero() {
    return (F2LinRngGeneric*) calloc(1, sizeof(F2LinRngGeneric));
}

F2LinRngGeneric* f2lin_rng_generic_init() {
    F2LinRngGeneric* rng = (F2LinRngGeneric* )calloc(1, sizeof(F2LinRngGeneric));
    rng->state[0] = 12323456ull;
    rng->state[1] = 54321ull;
    rng->state[2] = 0xa02b4c7dull;
    rng->state[3] = 0x94a3f7eeull;
    return rng;
}

F2LinRngGeneric* f2lin_rng_generic_init_seed(uint64_t seed) {
    F2LinRngGeneric* rng = f2lin_rng_generic_init_zero();
    rng->state[0] = seed;
    // use splitmix?
    return rng;
}

F2LinRngGeneric* f2lin_rng_generic_copy(F2LinRngGeneric *dest, 
        const F2LinRngGeneric *source) {
    dest->state[0] = source->state[0];
    dest->state[1] = source->state[1];
    dest->state[2] = source->state[2];
    dest->state[3] = source->state[3];
    return dest;
}

F2LinRngGeneric* f2lin_rng_generic_add(F2LinRngGeneric* lhs, 
        const F2LinRngGeneric* rhs) {
    lhs->state[0] ^= rhs->state[0];
    lhs->state[1] ^= rhs->state[1];
    lhs->state[2] ^= rhs->state[2];
    lhs->state[3] ^= rhs->state[3];
    return lhs;
}

uint64_t f2lin_rng_generic_next_state(F2LinRngGeneric* rng) {
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

uint64_t f2lin_rng_generic_gen64(F2LinRngGeneric* rng) {
    uint64_t* s = rng->state;
    const uint64_t result = rotl(s[0] + s[3], 23) + s[0];
    f2lin_rng_generic_next_state(rng);
    return result;
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

#ifdef TEST
int f2lin_rng_generic_compare_state(F2LinRngGeneric* lhs, F2LinRngGeneric* rhs) {
    return lhs->state[0] == rhs->state[0] &&
           lhs->state[1] == rhs->state[1] && 
           lhs->state[2] == rhs->state[2] &&   
           lhs->state[3] == rhs->state[3]; 
}
#endif
