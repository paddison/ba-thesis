/*
 * The goal of this benchmark is to see how the jump algorithm
 * compares with a very simple (inline) implementation of a random number generator.
 */

#include "mpi.h"
#include <stdlib.h>
#include <stdint.h>
#include "bench.h"
#include <stdio.h>

#define BUF_SIZE 100

static inline
uint64_t next(uint64_t state) {
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    return state;
}

static
void write_results(char exec_name[static 1], size_t N, unsigned long long buf[N], double results[N]) {
    char* fname;
    FILE* f;

    asprintf(&fname, "%s.csv", exec_name);
    f = fopen(fname, "a");
    fprintf(f, "n_numbers,time\n");

    for (size_t i = 0; i < N; ++i) {
        fprintf(f, "%llu,%5.2e\n", buf[i], results[i]);
    }

    fclose(f);
    free(fname);
}

static
double bench_iter(size_t iterations, size_t repetitions, unsigned long long arg) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    uint64_t state = 0x1232343456ull;

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            /* the thing we want to benchmark */
            for (size_t j = 0; j < arg; ++j) state = next(state);
        }
        times[1] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double res = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    FILE* f = fopen("/dev/null", "w");
    fprintf(f, "%llu\n", state);
    fclose(f);

    f2lin_bench_bmpi_destroy(&bmpi);

    return res;
}


int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);
    
    size_t iterations, repetitions, n_args = argc - 3;
    int rank;
    unsigned long long buf[BUF_SIZE];

    if (argc < 3) return EXIT_FAILURE;
    if (argc > BUF_SIZE + 3) return EXIT_FAILURE;

    iterations = strtoul(argv[1], 0, 10); 
    repetitions = strtoul(argv[2], 0, 10); 

    f2lin_bench_parse_argv(argc, &argv[3], buf);
    double results[n_args];
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (size_t i = 0; i < n_args; ++i) {
        /* run benchmark */
        double avg = bench_iter(iterations, repetitions, buf[i]);
        results[i] = avg;
        if (rank == 0) printf("jump: %llu\ttime: %5.2e\n", buf[i], avg);
    }

    if (rank == 0) write_results(argv[0], n_args, buf, results);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
