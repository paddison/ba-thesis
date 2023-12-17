#ifndef RNG_GENERIC_MT_H
#define RNG_GENERIC_MT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../lib/mt/mt.h"

#define XOR64_RNG_STATE_SIZE 19937

typedef struct Xor64RngGeneric Xor64RngGeneric;

struct Xor64RngGeneric {
    MT mt;
};

#ifdef __cplusplus
}
#endif
#endif
