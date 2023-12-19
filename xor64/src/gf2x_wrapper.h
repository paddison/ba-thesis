#ifndef GF2X_WRAPPER_H
#define GF2X_WRAPPER_H

#ifdef __cplusplus

#include "NTL/GF2X.h"
using namespace NTL;

extern "C" {
#else

typedef struct GF2X GF2X;
typedef struct GF2XModulus GF2XModulus;

#endif

/*-------------------------------------------------------------
 * Functionality for NTL::GF2x needed to calculate polynomials.
 ------------------------------------------------------------*/

GF2X* GF2X_zero_init();

void GF2X_zero_destroy(GF2X* p);

long GF2X_coeff(const GF2X* p, long i);

void GF2X_SetCoeff(GF2X* p, long i, long a);

long GF2X_deg(const GF2X* p);

void GF2X_print(const GF2X* x);


GF2XModulus* GF2XModulus_zero_init();

void GF2XModulus_destroy(GF2XModulus* F);

void GF2XModulus_build(GF2XModulus* F, const GF2X* f);

void GF2X_PowerMod(GF2X* x, const GF2X* a, const long e, const GF2XModulus* F);

#ifdef __cplusplus
}
#endif
#endif
