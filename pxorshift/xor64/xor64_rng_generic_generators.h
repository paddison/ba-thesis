#if defined (XorMT)
#warning "using mersenne twister as random number generator"
#else
#warning "using 64 bit xorshift as random number generator"
#include "xor64_rng_generic_64.h"
#endif
