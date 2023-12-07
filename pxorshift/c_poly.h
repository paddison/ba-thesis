#ifndef C_POLY_H
#define C_POLY_H

#include <stdlib.h>

typedef struct PolyGF2 PolyGF2;

typedef uint64_t PolyRaw;

struct PolyGF2 {
    PolyRaw* p;
    size_t cap;
    size_t deg;
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
    size_t m; // m is the length
    uint16_t hm1;
};


PolyGF2* poly_gf2_init(PolyGF2* p, size_t deg);

/**
 * Returns the degree of the polynomial @param p.
 */
size_t poly_gf2_deg(PolyGF2* p);

/**
 * Gets the coefficient at x^@param i of @param p.
 */
uint8_t poly_gf2_coeff(PolyGF2* p, size_t i); 

/**
 * Decomposes a PolyGF2 into subparameters for the sliding window
 * algorithm.
 */
PolyGF2Decomp poly_gf2_decomp(PolyGF2* p, uint16_t q);

#endif
