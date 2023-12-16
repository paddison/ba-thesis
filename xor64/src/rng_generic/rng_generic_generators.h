#if defined (XorMT)
#warning "using mersenne twister as random number generator"
#include "rng_generic_mt.h"
#else
#warning "using 64 bit xorshift as random number generator"
#include "rng_generic_64.h"
#endif
