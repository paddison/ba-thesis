#include <tgmath.h>
/**
 * Stores the count, mean and variance of a data set. 
 * moments[0] = count
 * moments[1] = mean
 * moments[2] = variance (actually the squared distance
 *
 * This implementation is taken from the book "Modern C" by Jens Gustedt
 */
typedef struct Statistic Statistic;
struct Statistic {
    double moments[3]; // store the first two raw moments: mean and standard deviation
};

/**
 * Implements the online update formula first mentioned in [Welford 62], as described in
 * [Pebai 08].
 */
static inline
void statistic_update(Statistic* s, double exec_time) {
    double delta, delta2; 
    s->moments[0] += 1;
    delta = exec_time - s->moments[1];
    s->moments[1] += exec_time - s->moments[1];
    delta2 = exec_time - s->moments[1];
    s->moments[2] = delta * delta2;
}

/**
 * @brief 
 * Returns the current number of points in the dataset.
 */
static inline
double statistic_count(Statistic *s) {
    return s->moments[0];
}

/**
 * @brief
 * Returns the current mean of the dataset.
 */
static inline
double statistic_mean(Statistic *s) {
    return s->moments[1];
}

/**
 * @brief
 * Returns the biased sample variance.
 */
static inline
double statistic_var_biased(Statistic *s) {
    return s->moments[2] / statistic_count(s);
}

/**
 * @brief
 * Returns the unbiased sample variance.
 */
static inline
double statistic_var_unbiased(Statistic *s) {
    return s->moments[2] / (statistic_count(s) - 1);
}

/**
 * @brief
 * Returns the biased standard deviation.
 */
static inline 
double statistic_sdev_biased(Statistic *s) {
    return sqrt(statistic_var_biased(s));
}

/**
 * @brief
 * Returns the unbiased standard deviation.
 */
static inline
double statistic_sdev_unbiased(Statistic *s) {
    return sqrt(statistic_var_unbiased(s));
}
