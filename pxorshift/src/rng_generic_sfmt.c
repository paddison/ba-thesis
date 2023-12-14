#include <assert.h>
#include <string.h>
#include "rng_generic_sfmt.h"

#define SEED 123456ul
#define WARM_UP 70000ul

/*
 * According to http://www.iro.umontreal.ca/~lecuyer/myftp/papers/lfsr04.pdf
 * it is advised to "warm up" mersenne twister, by generating 70k random numbers
 * directly after seeding it.
 */
static inline void warm_up(sfmt_t* sfmt) {
    for (size_t i = 0; i < WARM_UP; ++i) sfmt_genrand_uint64(sfmt); 
}

RngGeneric* rng_generic_init(RngGeneric* rng) {
    sfmt_init_gen_rand(&rng->sfmt, SEED);
    //warm_up(&rng->sfmt);

    return rng;
}

RngGeneric* rng_generic_copy(RngGeneric* dest, const RngGeneric* source) {
    memcpy(&dest->sfmt.state, source->sfmt.state, sizeof(w128_t) * SFMT_N) ;
    dest->sfmt.idx = source->sfmt.idx;
    return dest;
}

RngGeneric* rng_generic_add(RngGeneric* lhs, const RngGeneric* rhs) {
    // mersenne twister's state stores a pointer to 32 bit words in its
    // internal state. this means the states of lhs and rhs might be at 
    // different points in the rotation. when adding the states, we need 
    // to make sure that we use the right rotation
    //
    // SFMT_N64 is the number of 64 bit words

    // we add the states together in 64 bit words. the pointer to the internal
    // state needs to be even and divided by 2

    // store indice extra

    const size_t plhs = lhs->sfmt.idx / 2, prhs = rhs->sfmt.idx / 2, len = SFMT_N64;
    size_t i = 0;
    // represent internal state as 32 bit words
    uint64_t* slhs = (uint64_t*) &lhs->sfmt.state[0].u[0];
    const uint64_t* srhs = (uint64_t*) &rhs->sfmt.state[0].u[0];

    if (prhs >= plhs) {
        for (; i < len - prhs; ++i) {
            slhs[i + plhs] ^= srhs[i + prhs];
        }
        for (; i < len - plhs; ++i) { 
            slhs[i + plhs] ^= srhs[i + prhs - len];
        }
    } else {
        for (; i < len - plhs; ++i) {
            slhs[i + plhs] ^= srhs[i + prhs];
        }
        for (; i < len - prhs; ++i) {
            slhs[i + plhs - len] ^= srhs[i + prhs];
        }
    }
    for (; i < len; ++i) { 
        slhs[i + plhs - len] ^= srhs[i + prhs - len];
    }

    return lhs;
}

uint64_t rng_generic_step(RngGeneric* rng) {
    sfmt_gen_rand_all(&rng->sfmt);
    uint64_t* state = (uint64_t*) &rng->sfmt.state[0].u64[0];
    uint64_t n = state[rng->sfmt.idx / 2];
    printf("%llu\n", n);
    return  n;
}

RngGeneric* rng_generic_seed(RngGeneric* rng, void* seed) {
    sfmt_init_gen_rand(&rng->sfmt, *(uint64_t*) seed);
    //warm_up(&rng->sfmt);

    return rng;
}
