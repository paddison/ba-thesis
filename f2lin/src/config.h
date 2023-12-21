#ifndef CONFIG_H
#define CONFIG_H

#define Q_MAX 10
#define Q_DEFAULT 4
#define ALGORITHM_DEFAULT SLIDING_WINDOW_DECOMP

/**
 * Which algorithm to use for jumping ahead in the random number stream
 * Default is SLIDING_WINDOW.
 */
enum F2LinJumpAlgorithm {
    DEFAULT = 0, HORNER = 1, SLIDING_WINDOW = 2, SLIDING_WINDOW_DECOMP = 3, 
};

/**
 * Used for configuring the application.
 * q is the degree of the decomposition polynomials when using the sliding window method
 */
typedef struct F2LinConfig F2LinConfig;

struct F2LinConfig {
    enum F2LinJumpAlgorithm algorithm;
    unsigned q;
};

#endif