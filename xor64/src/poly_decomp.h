#ifndef PXOR64_POLY_H
#define PXOR64_POLY_H

#include <stdlib.h>
#include "gf2x_wrapper.h"

typedef struct Xor64PolyDecompParam Xor64PolyDecompParam;

struct Xor64PolyDecompParam {
   size_t d;
   uint16_t h;
};

typedef struct Xor64PolyDecomp Xor64PolyDecomp;

struct Xor64PolyDecomp {
    Xor64PolyDecompParam* params; 
    size_t cap;
    size_t m;           // m is the length
    uint16_t hm1;
};

/**
 * Initializes @param poly_decomp with a subpoly size of @param Q from @param jump_poly.
 *
 * @param decomp_poly can not be 0
 */
Xor64PolyDecomp* xor64_poly_decomp_init_from_gf2x(const GF2X* jump_poly, const size_t Q);

/**
 * Destroys @param decomp_poly, freeing any space allocated. 
 * @param decomp_poly has to be initialized with xor64_poly_decomp_init_from_gf2x() 
 * before calling this function.
 */
void xor64_poly_decomp_destroy(Xor64PolyDecomp* decomp_poly); 

#endif 
