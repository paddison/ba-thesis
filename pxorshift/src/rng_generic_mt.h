#ifndef RNG_GENERIC_MT_H
#define RNG_GENERIC_MT_H

#include "rng_generic.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../lib/mt/mt.h"

#define STATE_SIZE 19937

struct RngGeneric {
    MT mt;
};

#ifdef __cplusplus
}
#endif
#endif
