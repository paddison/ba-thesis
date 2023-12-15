#include "poly_decomp.hpp"

/* Forward Declarations */
static Xor64PolyDecomp* xor64_poly_decomp_init(Xor64PolyDecomp* poly_decomp, size_t q, size_t deg);
static int xor64_poly_decomp_add_param(Xor64PolyDecomp* poly_decomp, Xor64PolyDecompParam decomp_param); 

/* Header Implementations */
Xor64PolyDecomp* xor64_poly_decomp_init_from_gf2x(Xor64PolyDecomp* decomp_poly, const NTL::GF2X& jump_poly, const size_t Q) {
    int i = NTL::deg(jump_poly);

    xor64_poly_decomp_init(decomp_poly, Q, i);

    for (; i >= (int) Q; --i) {
        if (NTL::coeff(jump_poly, i) == 0) continue;
        
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
    *decomp_poly = { 0 };
}

/* Internal impolementations */
static Xor64PolyDecomp* xor64_poly_decomp_init(Xor64PolyDecomp* poly_decomp, size_t q, size_t deg) {
    if (q == 0 || poly_decomp == 0) return 0;

    poly_decomp->cap = deg > q ? deg / q : 16;
    poly_decomp->params = (Xor64PolyDecompParam *) realloc(poly_decomp->params, 
            sizeof(Xor64PolyDecompParam) * poly_decomp->cap);

    poly_decomp->m = 0;
    poly_decomp->hm1 = 0;

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

