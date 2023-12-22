#include "tools.h"
#include <stdio.h>
#include "gf2x_wrapper.h"
#include "rng_generic/rng_generic.h"

#define N 2

/* 
 * Forward Declarations 
 */
static 
int cmp_double(const void* a, const void* b);

static inline 
double median(const size_t len, const double data[len]);

static 
double median_absolute_deviation(const size_t len, const double data[len], 
                                          const double med);

static 
double remove_outliers(const size_t len, const double data[len], 
                                double sanitized[len], const double mad, 
                                const double med);

static 
double average(const size_t len, const double sanitized[len]);

/* 
 * Header implementations 
 */
double f2lin_tools_get_result(const size_t len, double data[len]) {
    // sort the data
    double med, mad;
    size_t sanitized_len;
    double sanitized[len];

    if (!len)  {
        fprintf(stderr, "data must contain at least 1 element");
        return -1;
    }

    qsort(data, len, sizeof(double), cmp_double);

    med = median(len, data);
    mad = median_absolute_deviation(len, data, med);
    sanitized_len = remove_outliers(len, data, sanitized, mad, med);

    return average(sanitized_len, sanitized);
}

GF2X* f2lin_tools_n_deg_poly_random(size_t deg) {
    GF2X* p = GF2X_zero_init();

    F2LinRngGeneric* rng = f2lin_rng_generic_init();
    GF2X_SetCoeff(p, 0, 1);
    GF2X_SetCoeff(p, deg, 1);
    for (size_t i = 1; i < deg; ++i) {
        GF2X_SetCoeff(p, i, f2lin_rng_generic_gen64(rng) & 1ull);
    }

    f2lin_rng_generic_destroy(rng);
    return p;
}

/* 
 * Internal implementations 
 */
int cmp_double(const void* a, const void* b) {
    if (*(double*)a > *(double*)b) return 1;
    else if (*(double*)a < *(double*)b) return -1;
    else return 0;
}

inline double median(const size_t len, const double data[len]) {
    if (len & 1ull) return data[len / 2];
    else return (data[len / 2] + data[(len + 1) / 2]) / 2;
}

double median_absolute_deviation(const size_t len, const double data[len], 
                                   const double med) {
    double abs_dev[len];

    for (size_t i = 0; i < len; ++i) {
        abs_dev[i] = data[i] > med ? data[i] - med : med - data[i];
    }

    qsort(abs_dev, len, sizeof(double), cmp_double);

    return median(len, abs_dev);
}

double remove_outliers(const size_t len, const double data[len], double sanitized[len], 
                         const double mad, const double med) {
    size_t sanitized_len = 0;
    size_t range = mad * N;

    for (size_t i = 0; i < len; ++i) {
        if (data[i] < med - range || data[i] > med + range) continue;
        sanitized[sanitized_len++] = data[i];
    }

    return sanitized_len;
}

double average(const size_t len, const double sanitized[len]) {
    double sum = 0;

    for (size_t i = 0; i < len; ++i) sum +=  sanitized[i];

    return sum / (double) len;
}

