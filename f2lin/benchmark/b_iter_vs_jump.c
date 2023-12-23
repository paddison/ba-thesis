/*
 * Test how long it takes to jump n numbers in comparison to calculate them iteratively.
 */

#include <stdlib.h>
#include <stdio.h>

#include "bench.h"
#include "f2lin.h"
#include "mpi.h"
#include "unistd.h"

typedef struct data data;
struct data {
    double ji;
    double jni;
    double iter;
};

static
void write_results(char exec_name[static 1], size_t N, 
                   unsigned long long jumps[N], data results[N]) {
    char* fname;
    FILE* f;

    asprintf(&fname, "%s.csv", exec_name);
    f = fopen(fname, "a");
    fprintf(f, "jump,jump_with_init,jump_no_init,iterative\n");

    for (size_t i = 0; i < N; ++i) {
        data p = results[i];
        fprintf(f, "%llu,%5.2e,%5.2e,%5.2e\n", jumps[i], p.ji, p.jni, p.iter);

    }
    fclose(f);
    free(fname);
}

static
double bench_iter(size_t iterations, size_t repetitions, unsigned long long jump_size) {
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

    f2lin_rng_destroy(rng);
    f2lin_bench_bmpi_destroy(&bmpi);

    return avg;
}

static
double bench_jump_no_init(size_t iterations, size_t repetitions, unsigned long long jump_size) {
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

    f2lin_bench_bmpi_destroy(&bmpi);
    f2lin_rng_destroy(rng);
    f2lin_jump_destroy(jump);

    return avg;
}

double bench_jump_with_init(size_t iterations, size_t repetitions, unsigned long long jump_size) {
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


    f2lin_bench_bmpi_destroy(&bmpi);
    f2lin_rng_destroy(rng);

    return avg;
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);

    unsigned long long buf[BUF_MAX];
    size_t iterations, repetitions, n_jumps = argc - 3;
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3) return EXIT_FAILURE;
    if (argc > BUF_MAX + 3) return EXIT_FAILURE;

    iterations = strtoul(argv[1], 0, 10);
    repetitions = strtoul(argv[2], 0, 10);

    if (iterations == -1 || repetitions == -1) return EXIT_FAILURE;

    f2lin_bench_parse_argv(argc, &argv[3], buf);
    data results[n_jumps];

    for (size_t i = 0; i < n_jumps; ++i) {
        results[i].ji = bench_jump_with_init(iterations, repetitions, buf[i]);
        results[i].jni = bench_jump_no_init(iterations, repetitions, buf[i]);
        results[i].iter = bench_iter(iterations, repetitions, buf[i]);
        
        if (rank == 0) printf("jump: %llu\tji: %5.2e\tjni: %5.2e\titer: %5.2e\n",
                              buf[i], results[i].ji, results[i].jni, results[i].iter);
    }

    if (rank == 0) write_results(argv[0], n_jumps, buf, results);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
