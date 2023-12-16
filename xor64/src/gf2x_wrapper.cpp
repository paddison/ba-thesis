#include "gf2x_wrapper.h"

GF2X* GF2X_zero_init() {
    return new GF2X();
}

void GF2X_zero_destroy(GF2X* p) {
    delete p;
}

long GF2X_coeff(const GF2X* a, long i) {
    return rep(coeff(*a, i));
}

void GF2X_SetCoeff(GF2X* p, long i, long a) {
    a = a & 1;
    SetCoeff(*p, i, a);
}

long GF2X_deg(const GF2X* p) {
    return deg(*p);
}


GF2XModulus* GF2XModulus_zero_init() {
    return new GF2XModulus();
}

void GF2XModulus_destroy(GF2XModulus* F) {
    delete F;
}

void GF2XModulus_build(GF2XModulus* F, const GF2X* f) {
    build(*F, *f);
}

void GF2X_PowerMod(GF2X* x, const GF2X* a, const long e, const GF2XModulus* F) {
    PowerMod(*x, *a, e, *F);
}
