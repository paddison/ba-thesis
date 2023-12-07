#include "c_poly.h"
#include <stdlib.h>
#include <stdio.h>

PolyGF2* poly_gf2_init(PolyGF2* p, size_t deg) {
    if (!p) p = (PolyGF2*) malloc(sizeof(PolyGF2));

    p->deg = deg;
    p->cap = (deg / 64) + 1;
    p->p = (PolyRaw*) calloc(sizeof(PolyRaw), p->cap);

    return p;
}

inline size_t poly_gf2_deg(PolyGF2* p) {
    return p->deg;
}

uint8_t poly_gf2_coeff(PolyGF2* p, size_t i) {
    if (i > p->deg) return 0;
    
    return (p->p[i / 64] >> (i % 64)) & 1;
}

size_t determine_gray_enumeration(size_t q, size_t i, PolyGF2* p) {
    size_t gray_enumeration = 0;

    for (size_t j = 0; j < q; ++j) {
        gray_enumeration = (gray_enumeration << 1) ^ poly_gf2_coeff(p, i - j - 1);
    }
    
    return gray_enumeration;
}

PolyGF2Decomp* poly_gf2_decomp_init(PolyGF2Decomp* pd, const PolyGF2* p, size_t q) {
    // assuming p is of length k and on average has k/2 non zero components
    // and also half of the components of a subpoly of size q are non zero,
    // the expected size will be k / (2 * (q / 2)) = k / q
    if (!p || !q) return 0;
    if (!pd) pd = (PolyGF2Decomp*) malloc(sizeof(PolyGF2Decomp));

    pd->cap = p->deg > q ? p->deg / q : 2;
    pd->params = (DecompParam*) malloc(sizeof(DecompParam) * pd->cap);
    pd->m = 0;
    pd->hm1 = 0;

    return pd; 
}

int poly_gf2_decomp_add_param(PolyGF2Decomp* pd, DecompParam dp) {
    if (!pd) return 0;

    if (pd->m == pd->cap) { 
        pd->cap <<= 1;
        DecompParam* params = (DecompParam*) realloc(pd->params, pd->cap * sizeof(DecompParam));
        if (!params) return 0;
        pd->params = params;
    }

    pd->params[pd->m++] = dp;

    return 1;
}

PolyGF2Decomp poly_gf2_decomp(PolyGF2* p, uint16_t q) {
    size_t i = poly_gf2_deg(p);
    PolyGF2Decomp pd;

    poly_gf2_decomp_init(&pd, p, q);

    while (!poly_gf2_coeff(p, i)) --i;

    for (; i > q; --i) {
        if (!poly_gf2_coeff(p, i)) continue;
        
        DecompParam dp = { 0 };
        dp.h = determine_gray_enumeration(q, i, p);
        dp.d = i - q;
        i -= q;
        if (!poly_gf2_decomp_add_param(&pd, dp)) {
            fprintf(stderr, "Unable to allocate space for new parameter");
        }
    }

    pd.hm1 = determine_gray_enumeration(i + 1, i + 1, p);

    return pd;
}
