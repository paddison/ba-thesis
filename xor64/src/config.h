#ifndef CONFIG_H
#define CONFIG_H

#define Q_MAX 10u
#define Q_DEFAULT 4u
#define ALGORITHM_DEFAULT SLIDING_WINDOW_DECOMP

/**
 * Which algorithm to use for jumping ahead in the random number stream
 * Default is SLIDING_WINDOW.
 */
enum JumpAlgorithm {
    DEFAULT = 0, HORNER = 1, SLIDING_WINDOW = 2, SLIDING_WINDOW_DECOMP = 3, 
};

/**
 * Used for configuring the application.
 * q is the degree of the decomposition polynomials when using the sliding window method
 */
typedef struct Xor64Config Xor64Config;

struct Xor64Config {
    enum JumpAlgorithm algorithm;
    unsigned q;
};

#endif
