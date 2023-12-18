#include <string.h>
#include <stdio.h>

#include "rng_generic.h"
#ifndef CALC_MIN_POLY 
#include "minpolymt.h"
#endif

#include "lib/mt/mt.h"

#define SEED 1234567
#define XOR64_RNG_STATE_SIZE 19937

struct Xor64RngGeneric {
    MT mt;
};

Xor64RngGeneric* xor64_rng_generic_init_zero() {
    return (Xor64RngGeneric* ) calloc(1, sizeof(Xor64RngGeneric));
}

Xor64RngGeneric* xor64_rng_generic_init() {
    Xor64RngGeneric* rng = (Xor64RngGeneric* ) calloc(1, sizeof(Xor64RngGeneric));
    mt_init_genrand64(&rng->mt, SEED);
    return rng;
}

Xor64RngGeneric* xor64_rng_generic_copy(Xor64RngGeneric* dest, const Xor64RngGeneric* source) {
    memcpy(dest->mt.mt, source->mt.mt, sizeof(uint64_t) * NN) ;
    dest->mt.mti = source->mt.mti;
    return dest;
}

Xor64RngGeneric* xor64_rng_generic_add(Xor64RngGeneric* lhs, const Xor64RngGeneric* rhs) {
    // mersenne twister's state stores a pointer to 32 bit words in its
    // internal state. this means the states of lhs and rhs might be at 
    // different points in the rotation. when adding the states, we need 
    // to make sure that we use the right rotation
    //
    // SFMT_N64 is the number of 64 bit words

    // we add the states together in 64 bit words. the pointer to the internal
    // state needs to be even and divided by 2

    // store indice extra

    const size_t plhs = lhs->mt.mti, prhs = rhs->mt.mti / 2, len = NN;
    size_t i = 0;
    // represent internal state as 64 bit words
    uint64_t* slhs = &lhs->mt.mt[0];
    const uint64_t* srhs =  &rhs->mt.mt[0];

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

uint64_t xor64_rng_generic_gen64(Xor64RngGeneric* rng) {
    return mt_genrand64_int64(&rng->mt);
}

void xor64_rng_generic_next_state(Xor64RngGeneric* rng) {
  int num;
  uint64_t y;
  static uint64_t mag02[2]={0x0ul, MATRIX_A};
  uint64_t* state = &rng->mt.mt[0];

  num = rng->mt.mti;
  if (num < NN-MM){
      y = (state[num] & UM) | (state[num+1] & LM);
      state[num] = state[num+MM] ^ (y >> 1) ^ mag02[y & 1UL];
      rng->mt.mti++;
  }
  else if (num < NN-1){
      y = (state[num] & UM) | (state[num+1] & LM);
      state[num] = state[num+(MM-NN)] ^ (y >> 1) ^ mag02[y & 1UL];
      rng->mt.mti++;
  }
  else if (num == NN-1){
      y = (state[NN-1] & UM) | (state[0] & LM);
      state[NN-1] = state[MM-1] ^ (y >> 1) ^ mag02[y % 1UL];
      rng->mt.mti = 0;
  }
}

Xor64RngGeneric* xor64_rng_generic_init_seed(uint64_t seed) {
    Xor64RngGeneric* rng = xor64_rng_generic_init_zero();
    mt_init_genrand64(&rng->mt, *(uint64_t*) seed);
    return rng;
}

long xor64_rng_generic_state_size() {
    return XOR64_RNG_STATE_SIZE;
}

void xor64_rng_generic_destroy(Xor64RngGeneric* rng) {
    free(rng);
}

#ifndef  CALC_MIN_POLY
char* xor64_rng_generic_min_poly() {
    return MIN_POLY;
}
#endif

#ifdef TEST
int xor64_rng_generic_compare_state(Xor64RngGeneric* lhs, Xor64RngGeneric* rhs) {
    return lhs->mt.mt[lhs->mt.mti] == rhs->mt.mt[lhs->mt.mti];
} 
#endif
