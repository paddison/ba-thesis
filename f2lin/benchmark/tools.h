#ifndef TOOLS_H
#define TOOLS_H

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdlib.h>
#include "gf2x_wrapper.h"

double f2lin_tools_get_result(const size_t len, double data[len]);

GF2X* f2lin_tools_n_deg_poly_random(size_t deg);


#ifdef __cplusplus 
}
#endif

#endif
