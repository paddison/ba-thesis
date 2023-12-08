#ifndef POLY_H
#define POLY_H

#include <stdlib.h>
#include "xorshift64.h"
#include "gray/gray.h"

/**
 * Which algorithm to use for jumping ahead in the random number stream
 * Default is POLY_SLIDING_WINDOW
 */
enum JumpAlgorithm {
    MATRIX, POLY_HORNER, POLY_SLIDING_WINDOW,
};

struct Poly {
    uint64_t p;
    size_t deg;
};

/**
 * Represents the state of a single thread. This includes parameters
 */
struct StateThread {
    size_t q;
    StateRng rng;
};

/**
 * Initializes the jump polynomials. This has to be called once at the beginning
 * of the program.
 */
//void init_jump(size_t jump_size);

StateThread* init_thread_state(StateThread* state);

/**
 * Jumps ahead in the stream. If empty state is 0, it will automatically
 * set up the jump with default parameters.
 */
StateThread* jump(StateThread* state);

// todo: add different data types

/**
 * Calculates the next random number in the stream.
 */
uint64_t next(StateThread* state);

#endif
