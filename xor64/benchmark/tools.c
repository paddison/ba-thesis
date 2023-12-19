#include "tools.h"
#include <stdio.h>

#define N 2

/* 
 * Forward Declarations 
 */
static int cmp_uint64_t(const void* a, const void* b);

static inline uint64_t median(const size_t len, const uint64_t data[len]);

static uint64_t median_absolute_deviation(const size_t len, const uint64_t data[len], 
                                          const uint64_t med);

static uint64_t remove_outliers(const size_t len, const uint64_t data[len], 
                                uint64_t sanitized[len], const uint64_t mad, 
                                const uint64_t med);

static double average(const size_t len, const uint64_t sanitized[len]);

/* 
 * Header implementations 
 */
double get_result(const size_t len, uint64_t data[len]) {
    // sort the data
    uint64_t med, mad;
    size_t sanitized_len;
    uint64_t sanitized[len];

    if (!len)  {
        fprintf(stderr, "data must contain at least 1 element");
        return -1;
    }

    qsort(data, len, sizeof(uint64_t), cmp_uint64_t);

    med = median(len, data);
    mad = median_absolute_deviation(len, data, med);
    sanitized_len = remove_outliers(len, data, sanitized, mad, med);

    return average(sanitized_len, sanitized);
}

/* 
 * Internal implementations 
 */
int cmp_uint64_t(const void* a, const void* b) {
    if (*(uint64_t*)a > *(uint64_t*)b) return 1;
    else if (*(uint64_t*)a < *(uint64_t*)b) return -1;
    else return 0;
}

inline uint64_t median(const size_t len, const uint64_t data[len]) {
    if (len & 1ull) return data[len / 2];
    else return (data[len / 2] + data[(len + 1) / 2]) / 2;
}

uint64_t median_absolute_deviation(const size_t len, const uint64_t data[len], 
                                   const uint64_t med) {
    uint64_t abs_dev[len];

    for (size_t i = 0; i < len; ++i) {
        abs_dev[i] = data[i] > med ? data[i] - med : med - data[i];
    }

    qsort(abs_dev, len, sizeof(uint64_t), cmp_uint64_t);

    return median(len, abs_dev);
}

uint64_t remove_outliers(const size_t len, const uint64_t data[len], uint64_t sanitized[len], 
                         const uint64_t mad, const uint64_t med) {
    size_t sanitized_len = 0;
    size_t range = mad * N;

    for (size_t i = 0; i < len; ++i) {
        if (data[i] < med - range || data[i] > med + range) continue;
        sanitized[sanitized_len++] = data[i];
    }

    return sanitized_len;
}

double average(const size_t len, const uint64_t sanitized[len]) {
    double sum = 0;

    for (size_t i = 0; i < len; ++i) sum += (double) sanitized[i];

    return sum / (double) len;
}

