#include "decomp_poly.hpp"
// default constructor
DecompPoly::DecompPoly() {
    this->params = nullptr;
    this->cap = 1;
    this->m = 0;
    this->hm1 = 0;
}
                     
DecompPoly::~DecompPoly() {
    if (this->cap > 0) {
        free(params);
        this->params = nullptr;
    }

    this->cap = 0;
    this->m = 0;
    this->hm1 = 0;
}


int DecompPoly::add_param(DecompParam param) {
    if (this->cap == 0) return 0;

    if (this->m == this->cap) { 
        this->cap <<= 1;
        DecompParam* params = (DecompParam*) realloc(this->params, this->cap * sizeof(DecompParam));
        if (!params) return 0;
        this->params = params;
    }

    this->params[this->m++] = param;

    return 1;
}

int DecompPoly::set_to_poly(const NTL::GF2X& poly, size_t q) {
    if (this->params) {
        delete this->params;
        this->params = nullptr;
        this->cap = 8;
    }
    int i = NTL::deg(poly);

    this->params = (DecompParam* ) malloc(sizeof(DecompParam) * (i / 64));

    for (; i >= q; --i) {
        if (NTL::coeff(poly, i) == 0) continue;
        
        DecompParam dp = { 0 };
        dp.h = determine_gray_enumeration(q, i, poly);
        dp.d = i - Q;
        i -= Q;
        if (!poly_gf2_decomp_add_param(&decomp_poly, dp)) {
            fprintf(stderr, "Unable to allocate space for new parameter");
        }
    }

    decomp_poly.hm1 = determine_gray_enumeration(i + 1, i + 1, poly);
    
}
