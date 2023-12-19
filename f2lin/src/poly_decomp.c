#include "poly_decomp.h"
#include "gray.h"

#include <stdlib.h>
#include <stdio.h>

/* Forward Declarations */
static F2LinPolyDecomp* f2lin_poly_decomp_init (size_t q, size_t deg);
static int f2lin_poly_decomp_add_param(F2LinPolyDecomp* poly_decomp, F2LinPolyDecompParam decomp_param); 

/* Header Implementations */
F2LinPolyDecomp* f2lin_poly_decomp_init_from_gf2x(const GF2X* jump_poly, const size_t Q) {
    int i = GF2X_deg(jump_poly);

    F2LinPolyDecomp* decomp_poly = f2lin_poly_decomp_init(Q, i);

    for (; i >= (int) Q; --i) {
        if (GF2X_coeff(jump_poly, i) == 0) continue;
        
        F2LinPolyDecompParam dp = { 0 };
        dp.h = f2lin_determine_gray_enumeration(Q, i, jump_poly);
        dp.d = i - Q;
        i -= Q;
        if (!f2lin_poly_decomp_add_param(decomp_poly, dp)) {
            fprintf(stderr, "Unable to allocate space for new parameter");
        }
    }

    decomp_poly->hm1 = f2lin_determine_gray_enumeration(i + 1, i + 1, jump_poly);

    return decomp_poly;
}

void f2lin_poly_decomp_destroy(F2LinPolyDecomp* decomp_poly) {
    free(decomp_poly->params);
    decomp_poly->m = 0;
    decomp_poly->params = 0;
    decomp_poly->cap = 0;
    decomp_poly->hm1 = 0;
}

/* Internal impolementations */
static F2LinPolyDecomp* f2lin_poly_decomp_init(size_t q, size_t deg) {
    F2LinPolyDecomp* poly_decomp = calloc(sizeof(F2LinPolyDecomp), 1);
    //F2LinPolyDecomp poly_decomp = { 0 };
    
    poly_decomp->cap = deg > q ? deg / q : 16;
    poly_decomp->params = (F2LinPolyDecompParam *) malloc(sizeof(F2LinPolyDecompParam) * poly_decomp->cap);

    return poly_decomp;
}

static int f2lin_poly_decomp_add_param(F2LinPolyDecomp* poly_decomp, F2LinPolyDecompParam decomp_param) {
    if (!poly_decomp) return 0;

    if (poly_decomp->m == poly_decomp->cap) { 
        poly_decomp->cap <<= 1;
        F2LinPolyDecompParam* params = (F2LinPolyDecompParam*) realloc(poly_decomp->params, poly_decomp->cap * sizeof(F2LinPolyDecompParam));
        if (!params) return 0;
        poly_decomp->params = params;
    }

    poly_decomp->params[poly_decomp->m++] = decomp_param;

    return 1;
}

