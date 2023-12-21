#include "tools.h"
#include <stdio.h>

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
double get_result(const size_t len, double data[len]) {
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

