#include "rng_generic_mt.h"
#include <string.h>
#include <stdio.h>
/*
 * According to http://www.iro.umontreal.ca/~lecuyer/myftp/papers/lfsr04.pdf
 * it is advised to "warm up" mersenne twister, by generating 70k random numbers
 * directly after seeding it.
 */

#define SEED 1234567

RngGeneric* rng_generic_init(RngGeneric* rng) {
    mt_init_genrand64(&rng->mt, SEED);
    return rng;
}

RngGeneric* rng_generic_copy(RngGeneric* dest, const RngGeneric* source) {
    memcpy(dest->mt.mt, source->mt.mt, sizeof(uint64_t) * NN) ;
    dest->mt.mti = source->mt.mti;
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

uint64_t rng_generic_step(RngGeneric* rng) {
    return mt_genrand64_int64(&rng->mt);
}

void rng_generic_next_state(RngGeneric* rng) {
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
  else {//if (num == NN-1){
      y = (state[NN-1] & UM) | (state[0] & LM);
      state[NN-1] = state[MM-1] ^ (y >> 1) ^ mag02[y % 1UL];
      rng->mt.mti = 0;
  }
}

RngGeneric* rng_generic_seed(RngGeneric* rng, void* seed) {
    mt_init_genrand64(&rng->mt, *(uint64_t*) seed);

    return rng;
}
