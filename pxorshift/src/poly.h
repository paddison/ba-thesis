#ifndef POLY_H
#define POLY_H

#include <stdlib.h>
#include "xorshift64.h"

/**
 * Which algorithm to use for jumping ahead in the random number stream
 * Default is POLY_SLIDING_WINDOW
 */
enum JumpAlgorithm {
    HORNER, SLIDING_WINDOW, SLIDING_WINDOW_DECOMP,
};

typedef struct StateThread StateThread;

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
void init_jump(size_t jump_size);

void set_jump_algorithm(enum JumpAlgorithm algorithm);

StateThread* init_thread_state(StateThread* state);

/**
 * Jumps ahead in the stream. If empty state is 0, it will automatically
 * set up the jump with default parameters.
 */
StateRng* jump(StateRng* state);

// todo: add different data types

/**
 * Calculates the next random number in the stream.
 */
uint64_t next(StateThread* state);

#endif
