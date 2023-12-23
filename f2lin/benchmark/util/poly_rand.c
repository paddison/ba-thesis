#include <stdlib.h>

#include "poly_rand.h"
#include "rng_generic/rng_generic.h"

GF2X* f2lin_poly_rand_init(size_t deg) {
    GF2X* p = GF2X_zero_init();

    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    GF2X_SetCoeff(p, 0, 1);
    GF2X_SetCoeff(p, deg, 1);
    for (size_t i = 1; i < deg; ++i) {
        GF2X_SetCoeff(p, i, f2lin_rng_generic_gen64(rng) & 1ull);
    }

    f2lin_rng_generic_destroy(rng);
    return p;
}

