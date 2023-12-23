#ifndef BENCH_H
#define BENCH_H

#ifdef __cplusplus
extern "C" {
#endif  

#include <stdlib.h>
#include <limits.h>

#define BUF_MAX 100
#define ROOT 0

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

#ifdef __cplusplus
void f2lin_bench_parse_argv(int argc, char **argv, unsigned long long *buf);
#else
void f2lin_bench_parse_argv(int argc, char *argv[argc - 3], unsigned long long buf[argc - 3]);
#endif

#ifdef __cplusplus
}
#endif  

#endif
