/*
 * This is a template with some basic functionality that is always required for
 * benchmark.
 *
 */

#include "mpi.h"
#include <stdlib.h>
#include "bench.h"

#define BUF_SIZE 100

static
void bench(size_t iterations, size_t repetitions, unsigned long long arg) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            /* the thing we want to benchmark */
        }
        times[1] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double res = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    if (bmpi.rank == bmpi.root) {
        /* print result */ 
    }

    f2lin_bench_bmpi_destroy(&bmpi);
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);
    
    size_t iterations, repetitions, n_args = argc - 3;
    unsigned long long buf[BUF_SIZE];

    if (argc < 3) return EXIT_FAILURE;
    if (argc > BUF_SIZE + 3) return EXIT_FAILURE;

    iterations = strtoul(argv[1], 0, 10); 
    repetitions = strtoul(argv[2], 0, 10); 

    f2lin_bench_parse_argv(argc, &argv[3], buf);


    for (size_t i = 0; i < n_args; ++i) {
        /* run benchmark */
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
