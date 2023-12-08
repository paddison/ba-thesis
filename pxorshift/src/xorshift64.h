#ifndef RNG_STATE_64_H
#define RNG_STATE_64_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
/**
 * Represents the current state of the rng, e.g. the current random number
 */
typedef struct StateRng64 StateRng64;
typedef struct StateRng64 StateRng;

struct StateRng64 {
    uint64_t s; 
};

StateRng64* state_rng_init(StateRng64* rng, void* seed);
StateRng64* state_rng_copy(StateRng64* dest, const StateRng64* source);
StateRng64* state_rng_add(StateRng64* lhs, const StateRng64* rhs);
StateRng64* step(StateRng64* rng); 
void set_params(uint64_t a, uint64_t b, uint64_t c);

#ifdef __cplusplus
}
#endif

#endif
