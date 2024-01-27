#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include "bench.h"
#include "prand48.h"

typedef struct data data;

struct data {
    double jp;
    double iter;
};

static 
void write_results(char exec_name[static 1], size_t N, unsigned long long jumps[N], 
                   data results[N]) {
    char* fname;
    FILE* f;

    asprintf(&fname, "%s.csv", exec_name);
    f = fopen(fname, "w");
    fprintf(f, "jumpsize,jump,iter\n");

    for (size_t i = 0; i < N; ++i) {
        fprintf(f, "%llu,%5.2e,%5.2e\n", 
               jumps[i], results[i].jp, results[i].iter);
    }

    fclose(f);
    free(fname);
}

static
double bench_jump(size_t iterations, size_t repetitions, unsigned long long jump) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];

    prand48_init();
    Prand48* prand = prand48_get();

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            /* the thing we want to benchmark */
            prand48_jump_abs(prand, jump);
        }
        times[1] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double res = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    prand48_destroy(prand);
    f2lin_bench_bmpi_destroy(&bmpi);
    
    return res;
}

static
double bench_iter(size_t iterations, size_t repetitions, unsigned long long jump) {
    F2LinBMPI bmpi = f2lin_bench_bmpi_init(repetitions);
    double times[2];
    uint16_t state[3];
    prand48_init();
    Prand48* prand = prand48_get();

    for (size_t rep = 0; rep < repetitions; ++rep) {
        times[0] = MPI_Wtime();
        for (size_t i = 0; i < iterations; ++i) {
            for (size_t j = 0; j < jump; ++j) prand48_next(prand); 
        }
        times[1] = MPI_Wtime();

        f2lin_bench_bmpi_update(&bmpi, rep, times[1] - times[0]);
    }

    double res = f2lin_bench_bmpi_eval(&bmpi) / (double) iterations;

    prand48_destroy(prand);
    f2lin_bench_bmpi_destroy(&bmpi);
    return res;
}

int main(int argc, char* argv[argc + 1]) {
    MPI_Init(&argc, &argv);
    
    size_t iterations, repetitions, n_args = argc - 3;
    unsigned long long buf[BUF_MAX];
    int rank;
    data *results;

    if (argc < 3) return EXIT_FAILURE;
    if (argc > BUF_MAX + 3) return EXIT_FAILURE;

    iterations = strtoul(argv[1], 0, 10); 
    repetitions = strtoul(argv[2], 0, 10); 

    if (iterations == -1 || repetitions == -1) return EXIT_FAILURE;

    f2lin_bench_parse_argv(argc, &argv[3], buf);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == ROOT) results = calloc(sizeof(data), n_args);

    for (size_t i = 0; i < n_args; ++i) {
        double avg_iter = bench_iter(iterations, repetitions, buf[i]);
        double avg_jump = bench_jump(iterations, repetitions, buf[i]);

        if (rank == ROOT) {
            printf("jump_size: %10llu\tjump: %5.2e\titer: %5.2e\n", 
                   buf[i], avg_jump, avg_iter);
            results[i].jp = avg_jump;
            results[i].iter = avg_iter;
        }
    }

    if (rank == ROOT) write_results(argv[0], n_args, buf, results);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
