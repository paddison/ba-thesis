#include "poly_decomp.h"
#include "gray.h"

#include <stdlib.h>
#include <stdio.h>

/* Forward Declarations */
static F2LinPolyDecomp* f2lin_poly_decomp_init (int q, int deg);

static F2LinPolyDecomp* f2lin_poly_decomp_init(int q, int deg) {
    F2LinPolyDecomp* pd = calloc(sizeof(F2LinPolyDecomp), 1);

    pd->cap = deg > q ? deg / q : 0;
    pd->h = (uint16_t*) calloc(sizeof(uint16_t), pd->cap);
    pd->d = (size_t*) calloc(sizeof(size_t), pd->cap);
    pd->m = 0;
    
    return pd;
}

// TODO test this
F2LinPolyDecomp* f2lin_poly_decomp_init_from_gf2x(const GF2X* jump_poly, const int Q) {
    int i = GF2X_deg(jump_poly);
    F2LinPolyDecomp* dp = f2lin_poly_decomp2_init_from_gf2x(jump_poly, i);

    for (; i >= Q; --i) {
        if (GF2X_coeff(jump_poly, i) == 0) continue;
        // TODO make typedefs for h and d
        // actually, we should never need to reallocate.
        dp->h[dp->m] = f2lin_determine_gray_enumeration(Q, i, jump_poly);
        i -= Q;
        dp->d[dp->m] = i;
        ++dp->m;
    }
    dp->hm1 = f2lin_determine_gray_enumeration(i + 1, i + 1, jump_poly);

    return dp;
}

void f2lin_poly_decomp2_destroy(F2LinPolyDecomp* pd) {
    free(pd->h);
    free(pd->d);
    free(pd);
    pd = 0;
}
