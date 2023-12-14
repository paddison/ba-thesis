#ifndef POLY_H
#define POLY_H

#ifdef __cplusplus 

extern "C" {

#endif
#include <stdlib.h>

#if defined(SFMT)
    #warning "including sfmt header"
    #include "rng_generic_sfmt.h"
#elif defined(MT64)
    #warning "including mt header"
    #include "rng_generic_mt.h"
#else
    #warning "including xor64 header"
    #include "rng_generic_xor64.h"
#endif

#include "gray.h"

/**
 * Which algorithm to use for jumping ahead in the random number stream
 * Default is SLIDING_WINDOW.
 */
enum JumpAlgorithm {
    DEFAULT = 0, HORNER = 1, SLIDING_WINDOW = 2, SLIDING_WINDOW_DECOMP = 3, 
};

/**
 * Used for configuring the application.
 * q is the degree of the decomposition polynomials when using the sliding window method
 */
typedef struct Config Config;

struct Config {
    enum JumpAlgorithm algorithm;
    size_t q;
};

typedef struct DecompParam DecompParam;

struct DecompParam {
   size_t d;
   uint16_t h;
};

typedef struct PolyGF2Decomp PolyGF2Decomp;

struct PolyGF2Decomp {
    DecompParam* params; 
    size_t cap;
    size_t m;           // m is the length
    uint16_t hm1;
};

void polynomials_init(Config* config);

void polynomials_init_jump(size_t jump);

RngGeneric* polynomials_jump(RngGeneric* rng);

#ifdef __cplusplus 

}

#endif

#endif
