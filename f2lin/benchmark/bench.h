#ifndef BENCH_H
#define BENCH_H

#ifdef __cplusplus
extern "C" {
#endif  

#include <stdlib.h>
#include <limits.h>


typedef struct F2LinBench F2LinBench;
typedef void fbench(F2LinBench);

typedef struct F2LinBMPI F2LinBMPI;
struct F2LinBMPI {
    int rank;
    int gsize;
    int root;
    size_t repetitions;
    double* times;
};

F2LinBMPI f2lin_bench_bmpi_init(size_t repetitions);

static inline
void f2lin_bench_bmpi_destroy(F2LinBMPI* bmpi) {
    free(bmpi->times);
}

static inline
void f2lin_bench_bmpi_update(F2LinBMPI*bmpi, size_t i, double time) {
        bmpi->times[i] = time;
}

double f2lin_bench_bmpi_eval(F2LinBMPI* bmpi); 

static inline
void f2lin_bench_parse_argv(int argc, char *argv[argc - 3], unsigned long long buf[argc - 3]) {
    // we ignore the first argument, which is the program name
    for (size_t i = 0; i < argc - 3; ++i) {
        unsigned long long n = strtoull(argv[i], 0, 10);
        if (n != ULLONG_MAX) buf[i] = n;
    }
}

#ifdef __cplusplus
}
#endif  

#endif
