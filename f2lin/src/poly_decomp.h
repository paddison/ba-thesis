#ifndef PXOR64_POLY_H
#define PXOR64_POLY_H

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "gf2x_wrapper.h"

typedef struct F2LinPolyDecompParam F2LinPolyDecompParam;

struct F2LinPolyDecompParam {
   size_t d;
   uint16_t h;
};

typedef struct F2LinPolyDecomp F2LinPolyDecomp;

struct F2LinPolyDecomp {
    F2LinPolyDecompParam* params; 
    size_t cap;
    size_t m;           // m is the length
    uint16_t hm1;
};

/**
 * Initializes @param poly_decomp with a subpoly size of @param Q from @param jump_poly.
 *
 * @param decomp_poly can not be 0
 */
F2LinPolyDecomp* f2lin_poly_decomp_init_from_gf2x(const GF2X* jump_poly, const size_t Q);

/**
 * Destroys @param decomp_poly, freeing any space allocated. 
 * @param decomp_poly has to be initialized with f2lin_poly_decomp_init_from_gf2x() 
 * before calling this function.
 */
void f2lin_poly_decomp_destroy(F2LinPolyDecomp* decomp_poly); 

#endif 
