/*
 * Test how long it takes to jump n numbers in comparison to calculate them iteratively.
 */

#include <stdlib.h>
#include <stdio.h>

#include "bench.h"
#include "f2lin.h"
#include "mpi.h"

void bench_iter(size_t iterations, size_t repetitions, unsigned long long jump_size) {
    F2LinRngGeneric* rng = f2lin_rng_init(); 
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime(); 
        for (size_t i = 0; i < iterations; ++i) {
            for (size_t  j = 0; j < jump_size; ++j) {
                f2lin_next_double(rng);
            }
        }
        times[1] = MPI_Wtime();
        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    if (bmpi.rank == bmpi.root) {
        printf("jump: %llu\ttime: %5.2es\n", jump_size, avg);
    }

    f2lin_rng_destroy(rng);
    f2lin_bench_bmpi_destroy(&bmpi);
}

void bench_jump_no_init(size_t iterations, size_t repetitions, unsigned long long jump_size) {
    F2LinRngGeneric* rng = f2lin_rng_init();
    F2LinJump* jump = f2lin_jump_init(jump_size, 0);
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            f2lin_jump(rng, jump);
        }
        times[1] = MPI_Wtime();
        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    if (bmpi.rank == bmpi.root) {
        printf("jump: %llu\ttime: %5.2es\n", jump_size, avg);
    }

    f2lin_bench_bmpi_destroy(&bmpi);
    f2lin_rng_destroy(rng);
    f2lin_jump_destroy(jump);
}

void bench_jump_with_init(size_t iterations, size_t repetitions, unsigned long long jump_size) {
    F2LinRngGeneric* rng = f2lin_rng_init();
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    F2LinJump* jumps[iterations];
    double times[2];

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            jumps[i] = f2lin_jump_init(jump_size, 0);
            f2lin_jump(rng, jumps[i]);
        }
        times[1] = MPI_Wtime();
        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);

        // clean up
        for (size_t i = 0; i < iterations; ++i) {
            f2lin_jump_destroy(jumps[i]);
        }
    }

    double avg = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    if (bmpi.rank == bmpi.root) {
        printf("jump: %llu\ttime: %5.2es\n", jump_size, avg);
    }

    f2lin_bench_bmpi_destroy(&bmpi);
    f2lin_rng_destroy(rng);
}
int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);

    unsigned long long buf[100];
    size_t iterations, repetitions, n_jumps = argc - 3;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3) return EXIT_FAILURE;
    if (argc > 103) return EXIT_FAILURE;

    iterations = strtoul(argv[1], 0, 10);
    repetitions = strtoul(argv[2], 0, 10);

    if (iterations == -1 || repetitions == -1) return EXIT_FAILURE;

    f2lin_bench_parse_argv(argc, &argv[3], buf);


    if (rank == 0) {
        printf("===========================================\n");
        printf("Jump with init:\n");
    }
    for (size_t i = 0; i < n_jumps; ++i) {
        bench_jump_with_init(iterations, repetitions, buf[i]);
    }

    if (rank == 0) {
        printf("===========================================\n");
        printf("Jump no init:\n");
    }
    for (size_t i = 0; i < n_jumps; ++i) {
        bench_jump_no_init(iterations, repetitions, buf[i]);
    }

    if (rank == 0) {
        printf("===========================================\n");
        printf("Iter:\n");
    }
    for (size_t i = 0; i < n_jumps; ++i) {
        bench_iter(iterations, repetitions, buf[i]);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
