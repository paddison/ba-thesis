#ifndef PXOR64_POLY_H
#define PXOR64_POLY_H

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "gf2x_wrapper.h"

typedef struct F2LinPolyDecomp F2LinPolyDecomp;
struct F2LinPolyDecomp {
    uint16_t* h;
    size_t* d;
    size_t m;
    uint16_t hm1;
    size_t cap;
};

/**
 * Initializes @param poly_decomp with a subpoly size of @param Q from @param jump_poly.
 *
 * @param decomp_poly can not be 0
 */
F2LinPolyDecomp* f2lin_poly_decomp_init_from_gf2x(const GF2X* jump_poly, const int Q);

/**
 * Destroys @param decomp_poly, freeing any space allocated. 
 * @param decomp_poly has to be initialized with f2lin_poly_decomp_init_from_gf2x() 
 * before calling this function.
 */
void f2lin_poly_decomp_destroy(F2LinPolyDecomp* decomp_poly); 

#endif 
