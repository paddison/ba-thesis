#include "poly_decomp.h"
#include "gray.h"

#include <stdlib.h>
#include <stdio.h>

/* Forward Declarations */
static Xor64PolyDecomp* xor64_poly_decomp_init (size_t q, size_t deg);
static int xor64_poly_decomp_add_param(Xor64PolyDecomp* poly_decomp, Xor64PolyDecompParam decomp_param); 

/* Header Implementations */
Xor64PolyDecomp* xor64_poly_decomp_init_from_gf2x(const GF2X* jump_poly, const size_t Q) {
    int i = GF2X_deg(jump_poly);

    Xor64PolyDecomp* decomp_poly = xor64_poly_decomp_init(Q, i);

    for (; i >= (int) Q; --i) {
        if (GF2X_coeff(jump_poly, i) == 0) continue;
        
        Xor64PolyDecompParam dp = { 0 };
        dp.h = determine_gray_enumeration(Q, i, jump_poly);
        dp.d = i - Q;
        i -= Q;
        if (!xor64_poly_decomp_add_param(decomp_poly, dp)) {
            fprintf(stderr, "Unable to allocate space for new parameter");
        }
    }

    decomp_poly->hm1 = determine_gray_enumeration(i + 1, i + 1, jump_poly);

    return decomp_poly;
}

void xor64_poly_decomp_destroy(Xor64PolyDecomp* decomp_poly) {
    free(decomp_poly->params);
    decomp_poly->m = 0;
    decomp_poly->params = 0;
    decomp_poly->cap = 0;
    decomp_poly->hm1 = 0;
}

/* Internal impolementations */
static Xor64PolyDecomp* xor64_poly_decomp_init(size_t q, size_t deg) {
    Xor64PolyDecomp* poly_decomp = calloc(sizeof(Xor64PolyDecomp), 1);
    //Xor64PolyDecomp poly_decomp = { 0 };
    
    poly_decomp->cap = deg > q ? deg / q : 16;
    poly_decomp->params = (Xor64PolyDecompParam *) malloc(sizeof(Xor64PolyDecompParam) * poly_decomp->cap);

    return poly_decomp;
}

static int xor64_poly_decomp_add_param(Xor64PolyDecomp* poly_decomp, Xor64PolyDecompParam decomp_param) {
    if (!poly_decomp) return 0;

    if (poly_decomp->m == poly_decomp->cap) { 
        poly_decomp->cap <<= 1;
        Xor64PolyDecompParam* params = (Xor64PolyDecompParam*) realloc(poly_decomp->params, poly_decomp->cap * sizeof(Xor64PolyDecompParam));
        if (!params) return 0;
        poly_decomp->params = params;
    }

    poly_decomp->params[poly_decomp->m++] = decomp_param;

    return 1;
}

